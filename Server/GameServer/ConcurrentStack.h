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
		_stack.push(std::move(value));
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
	 

//template<typename T>
//class LockFreeStack
//{
//	struct Node
//	{
//		Node(const T& value) : data(make_shared<T>(value)), next(nullptr)
//		{
//
//		}
//
//		shared_ptr<T> data;
//		shared_ptr<Node> next;
//	};
//
//public:
//	void Push(const T& value)
//	{
//		shared_ptr<Node> node = make_shared<Node>(value);
//		node->next = std::atomic_load(&_head);
//	
//		while (std::atomic_compare_exchange_weak(&_head, &node->next, node) == false)
//		{
//		}
//	}
//
//	shared_ptr<T> TryPop()
//	{
//		shared_ptr<Node> oldHead = std::atomic_load(&_head);
//
//		while (oldHead && std::atomic_compare_exchange_weak(&_head, &oldHead, oldHead->next) == false)
//		{
//		}
//
//		if (nullptr == oldHead)
//		{
//			return shared_ptr<T>();
//		}
//
//		return oldHead->data;
//	}
//
//private:
//	shared_ptr<Node> _head;
//};

template<typename T>
class LockFreeStack
{
	struct Node;

	struct CountedNodePtr
	{
		// 참조 횟수를 관리
		int32 externalCount = 0; // 일반 포인터가 아니다 외부적으로 참조 횟수를 같이 세는 포인터
		Node* ptr = nullptr;
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value))
		{
		}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next; // 포인터와 포인터 참조횟수를 관리
	};

public:
	// [][][][][][][]
	// [head]
	// 노드 추가해서 헤드가 새로 생성한 노드를 가리키기
	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1; // 여기까진 stack memory, 스레드별로 다름
		// []
		node.ptr->next = _head; // 시도하고, 아래와 같이 경합하여 포인터를 가리킴
		while (_head.compare_exchange_weak(node.ptr->next, node) == false)
		{
		}
	
	}

	// [][][][][][]
	// [head]
	shared_ptr<T> TryPop()
	{
		CountedNodePtr oldHead = _head;
		while (true)
		{
			// 참조권 획득 (externalCount를 현 시점 기준 +1 한 애가 이김)
			IncreaseHeadCount(oldHead);

			// 최소한 externalCount >= 2 일테니 삭제X (안전하게 접근할 수 있는)
			Node* ptr = oldHead.ptr;

			// 데이터 없음
			if (nullptr == ptr)
			{
				return shared_ptr<T>();
			}

			// 소유권 획득 (ptr->next로 head를 바꿔치기 한 애가 이김)
			if (_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res.swap(ptr->data);

				// external : 시작은 1 -> 2(나+1) -> 4(나+1, 남+2) , 번호표
				// internal : 시작은 0 -> 2 -> 1 (else로 깎임), (마지막 사람 불끄기 체크용)
				const int32 countIncrease = oldHead.externalCount - 2;
				if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease) // 나만 남으면 삭제처리
				{
					delete ptr;
				}
				return res;
			}
			else if (ptr->internalCount.fetch_sub(1) == 1) // 참조권은 얻었으나, 소유권은 실패, (빼기 적용됨)
			{
				// 뒷수습은 내가 한다 (나만 남으면 삭제처리)
				delete ptr;
			}

		}
	}

	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true) // 성공할 때까지 시도
		{
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++; // 증가만 하는 참조권카운트(번호표)

			if (_head.compare_exchange_strong(oldCounter, newCounter)) // head가 가리키는것 성공시
			{
				oldCounter.externalCount = newCounter.externalCount; // 호출된 함수에 넘어갈 oldCounter에 적용
				break;
			}
		}
	}

private:
	atomic<CountedNodePtr> _head; // 포인터와 참조 횟수를 아토믹하게 관리
};