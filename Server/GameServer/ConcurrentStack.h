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
	// 1) �� ��带 �����
	// 2) �� ����� next = head
	// 3) head = �� ���
	void Push(const T& value)
	{
		Node* node = new Node(value);
		node->next = _head;
	
		while (_head.compare_exchange_weak(node->next, node) == false)
		{
			// node->next = _head;
		}

		// �� ���̿� ��ġ�� ���ϸ�?
		//_head = node;

	}

	// 1) head �б�
	// 2) head->next �б�
	// 3) head = head->next
	// 4) data �����ؼ� ��ȯ
	// 5) ������ ��带 ����
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

	// 1) ������ �и�
	// 2) Count üũ
	// 3) �� ȥ�ڸ� ����
	void TryDelete( Node* oldHead )
	{
		// �� �ܿ� ���� �ִ���?
		if (_popCount == 1)
		{
			// �� ȥ�ڳ�?

			// �̿� ȥ���ΰ�, ���� ����� �ٸ� �����͵鵵 �����غ���
			Node* node = _pendingList.exchange(nullptr);

			if (--_popCount == 0) // ������ϰ� �̷����! ���� ���ϴ°� �ƴ϶� ���鼭 ������� ���°��� ���������� �̷����
			{
				// ����� �ְ� ���� -> ���� ����
				// �����ͼ� �����, ������ �����ʹ� �и��ص� ����~!
				DeleteNodes(node);
			}
			else if(node)
			{
				// ���� ���������� �ٽ� ���� ����.
				ChangePendingNodeList(node);
			}
			 
			// �� �����ʹ� ����
			delete oldHead;

		}
		else
		{
			// ���� �ֳ�? �׷� ���� �������� �ʰ�, ���� ���ุ
			ChangePendingNodeList(oldHead);
			--_popCount;
		}
	
	}

	// [][][][][][] -> [][][][] 

	void ChangePendingNodeList(Node* first, Node* last) // �߰��� �ƹ��� �ȳ������ٸ� �̾��ش� + _pendingList�� �� ó���� ����Ű��
	{
		last->next = _pendingList;
		while (_pendingList.compare_exchange_weak(last->next, first) == false)
		{

		}

	}

	void ChangePendingNodeList(Node* node) // �� ���� ����Ű��, �� ������ ��带 ã���ִ� helper�Լ�
	{
		Node* last = node;
		while (last->next)
		{
			last = last->next;
		}

		ChangePendingNodeList(node, last);
	}

	void ChangePendingNode(Node* node) // �ϳ�¥��
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
	
	atomic<uint32> _popCount = 0; // Pop�� �������� ������ ����
	atomic<Node*> _pendingList; // ���� �Ǿ�� �� ���� (ù��° ���)

};