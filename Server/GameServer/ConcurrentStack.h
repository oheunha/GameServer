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
	 

template<typename T>
class LockFreeStack
{
	struct Node
	{
		Node(const T& value) : data(value), next(nullptr)
		{

		}

		T data;
		Node* next;
	};

public:
	// 1) 새 노드를 만들고
	// 2) 새 노드의 next = head
	// 3) head = 새 노드
	void Push(const T& value)
	{
		Node* node = new Node(value);
		node->next = _head;
	
		while (_head.compare_exchange_weak(node->next, node) == false)
		{
			// node->next = _head;
		}

		// 이 사이에 새치기 당하면?
		//_head = node;

	}

	// 1) head 읽기
	// 2) head->next 읽기
	// 3) head = head->next
	// 4) data 추출해서 반환
	// 5) 추출한 노드를 삭제
	bool TryPop(T& value)
	{
		++_popCount;

		Node* oldHead = _head;
		
		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
		}
		
		if (oldHead == nullptr)
		{
			--_popCount;

			return false;
		}
		

		value = oldHead->data;
		TryDelete(oldHead);
		return true;

	}

	// 1) 데이터 분리
	// 2) Count 체크
	// 3) 나 혼자만 삭제
	void TryDelete( Node* oldHead )
	{
		// 나 외에 누가 있는지?
		if (_popCount == 1)
		{
			// 나 혼자네?

			// 이왕 혼자인거, 삭제 예약된 다른 데이터들도 삭제해보자
			Node* node = _pendingList.exchange(nullptr);

			if (--_popCount == 0) // 아토믹하게 이루어짐! 빼고 겟하는게 아니라 빼면서 결과값을 뱉어내는것이 원자적으로 이루어짐
			{
				// 끼어든 애가 없음 -> 삭제 진행
				// 이제와서 끼어들어도, 어차피 데이터는 분리해둔 상태~!
				DeleteNodes(node);
			}
			else if(node)
			{
				// 누가 끼어들엇으니 다시 갖다 놓자.
				ChangePendingNodeList(node);
			}
			 
			// 내 데이터는 삭제
			delete oldHead;

		}
		else
		{
			// 누가 있네? 그럼 지금 삭제하지 않고, 삭제 예약만
			ChangePendingNodeList(oldHead);
			--_popCount;
		}
	
	}

	// [][][][][][] -> [][][][] 

	void ChangePendingNodeList(Node* first, Node* last) // 중간에 아무도 안끼어들었다면 이어준다 + _pendingList의 맨 처음을 가리키기
	{
		last->next = _pendingList;
		while (_pendingList.compare_exchange_weak(last->next, first) == false)
		{

		}

	}

	void ChangePendingNodeList(Node* node) // 맨 앞을 가리키며, 맨 마지막 노드를 찾아주는 helper함수
	{
		Node* last = node;
		while (last->next)
		{
			last = last->next;
		}

		ChangePendingNodeList(node, last);
	}

	void ChangePendingNode(Node* node) // 하나짜리
	{
		ChangePendingList(node, node);
	}

	static void DeleteNodes(Node* node)
	{
		while (node)
		{
			Node* next = node->next;
			delete node;
			node = next;

		}
	}

private:
	// [][][][][][]
	// [head]
	atomic<Node*> _head;
	
	atomic<uint32> _popCount = 0; // Pop을 실행중인 쓰레드 개수
	atomic<Node*> _pendingList; // 삭제 되어야 할 노드들 (첫번째 노드)

};