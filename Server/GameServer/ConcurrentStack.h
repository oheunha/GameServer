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
		_condVar.notify_one(); // ����ϰ� �ִ� �� �ϳ� ������, �ñ׳�
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

	void WaitPop(T& value) // ������ ������ ���� ���
	{
		unique_lock<mutex> lock(_mutex); // condition_variable�� wait�Ҷ� ���������� ���� Ǯ���⵵ �ϱ⿡ ����ũ���� ���
		_condVar.wait(lock, [this]{ return _stack.empty() == false; }); // ���� �������� ������ ���� Ǯ�� ����

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
		// ���� Ƚ���� ����
		int32 externalCount = 0; // �Ϲ� �����Ͱ� �ƴϴ� �ܺ������� ���� Ƚ���� ���� ���� ������
		Node* ptr = nullptr;
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value))
		{
		}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next; // �����Ϳ� ������ ����Ƚ���� ����
	};

public:
	// [][][][][][][]
	// [head]
	// ��� �߰��ؼ� ��尡 ���� ������ ��带 ����Ű��
	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1; // ������� stack memory, �����庰�� �ٸ�
		// []
		node.ptr->next = _head; // �õ��ϰ�, �Ʒ��� ���� �����Ͽ� �����͸� ����Ŵ
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
			// ������ ȹ�� (externalCount�� �� ���� ���� +1 �� �ְ� �̱�)
			IncreaseHeadCount(oldHead);

			// �ּ��� externalCount >= 2 ���״� ����X (�����ϰ� ������ �� �ִ�)
			Node* ptr = oldHead.ptr;

			// ������ ����
			if (nullptr == ptr)
			{
				return shared_ptr<T>();
			}

			// ������ ȹ�� (ptr->next�� head�� �ٲ�ġ�� �� �ְ� �̱�)
			if (_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res.swap(ptr->data);

				// external : ������ 1 -> 2(��+1) -> 4(��+1, ��+2) , ��ȣǥ
				// internal : ������ 0 -> 2 -> 1 (else�� ����), (������ ��� �Ҳ��� üũ��)
				const int32 countIncrease = oldHead.externalCount - 2;
				if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease) // ���� ������ ����ó��
				{
					delete ptr;
				}
				return res;
			}
			else if (ptr->internalCount.fetch_sub(1) == 1) // �������� �������, �������� ����, (���� �����)
			{
				// �޼����� ���� �Ѵ� (���� ������ ����ó��)
				delete ptr;
			}

		}
	}

	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true) // ������ ������ �õ�
		{
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++; // ������ �ϴ� ������ī��Ʈ(��ȣǥ)

			if (_head.compare_exchange_strong(oldCounter, newCounter)) // head�� ����Ű�°� ������
			{
				oldCounter.externalCount = newCounter.externalCount; // ȣ��� �Լ��� �Ѿ oldCounter�� ����
				break;
			}
		}
	}

private:
	atomic<CountedNodePtr> _head; // �����Ϳ� ���� Ƚ���� ������ϰ� ����
};