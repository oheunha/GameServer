#pragma once

#include <mutex>

template<typename T>
class LockStack
{
public:
	LockStack() {}

	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move());
		_condVar.notify_one(); // 대기하고 있는 애 하나 꺼내기, 시그널
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty())
			return false;
		
		// empty -> top -> pop
		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	void WaitPop(T& value) // 데이터 있을때 까지 대기
	{
		unique_lock<mutex> lock(_mutex); // condition_variable의 wait할때 내부적으로 락이 풀리기도 하기에 유니크락을 사용
		_condVar.wait(lock, [this]{ return _stack.empty() == false; }); // 조건 만족하지 않으면 락을 풀고 잠든다

		value = std::move(_stack.top()); 
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;

};
	 

