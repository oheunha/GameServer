#include "pch.h"
#include "LockFreeStack.h"
/*
void InitializeHead(SListHeader* header)
{
	header->next = nullptr;
}
void PushEntrySList(SListHeader* header, SListEntry* entry)
{
	entry->next = header->next;
	header->next = entry;
}

SListEntry* PopEntrySList(SListHeader* header)
{
	SListEntry* first = header->next;

	if (first != nullptr)
	{
		header->next = first->next;
	}
	return first;
}
*/

/*
2차시도
*/
/*
void InitializeHead(SListHeader* header)
{
	header->next = nullptr;
}

void PushEntrySList(SListHeader* header, SListEntry* entry)
{
	entry->next = header->next;
											//  [비교1]              [desire,최종원하는것]     [expect,비교대상2]
	while (::InterlockedCompareExchange64((int64*)&header->next, (int64)entry, (int64)entry->next) == 0)
	{	// [비교1] 과 [비교2]가 같으면  [최종원하는것]이 [비교1]에 들어간다.
		// 실패하면 0을 리턴한다.


	}

	header->next = entry;
}

// [][][]
// Header[ next ]
SListEntry* PopEntrySList(SListHeader* header)
{
	SListEntry* expected = header->next;

	// ABA Problom
	// [Header(6000, 1)]

	// 만약에 Header가 5000이라면, Header에다 6000을 넣어줘!
	// [5000]->[6000]->[7000]
	// [Header]
	// 상태가 바뀌었는데 상태가 바뀐것을 알지 못하고 그대로 진행될 수 있다.
	while (expected && ::InterlockedCompareExchange64((int64*)&header->next, (int64)expected->next, (int64)expected) == 0)
	{

	}

	return expected;
}
*/

/*
	3차시도
*/
void InitializeHead(SListHeader* header)
{
	header->alignment = 0;
	header->region = 0;
}

void PushEntrySList(SListHeader* header, SListEntry* entry)
{
	SListHeader expected = {};
	SListHeader desired = {};

	
	// 최하위 4비트가 0000이면 16바이트로 딱떨어짐
	// 16바이트 정렬, 4비트 날림
	desired.HeaderX64.next = (((uint64)entry) >> 4);

	while (true)
	{
		expected = *header;

		// 이 사이에 변경될 수 있다
		// 4비트 날린것을 next에 복원
		entry->next = (SListEntry*)(((uint64)expected.HeaderX64.next) << 4);
		desired.HeaderX64.depth = expected.HeaderX64.depth + 1;
		desired.HeaderX64.sequence = expected.HeaderX64.sequence + 1;
		//-> 나만 유효한 값
	
		// 128비트
		if (::InterlockedCompareExchange128((int64*)header, desired.region, desired.alignment, (int64*)&expected) == 1)
			break;
		// 1리턴이면 성공.
	}

}

// [][][]
// Header[ next ]
SListEntry* PopEntrySList(SListHeader* header)
{
	SListHeader expected = {};
	SListHeader desired = {};
	SListEntry* entry = nullptr;

	while (true)
	{
		expected = *header;
		entry = (SListEntry*)(((uint64)expected.HeaderX64.next) << 4);
		if (entry == nullptr)
			break;

		// Use-After-Free 생길 수 있음.
		desired.HeaderX64.next = ((uint64)entry->next) >> 4; // 밀어넣는다.
		// 겹치지 않는다.
		desired.HeaderX64.depth = expected.HeaderX64.depth - 1;
		desired.HeaderX64.sequence = expected.HeaderX64.sequence + 1;

		if (::InterlockedCompareExchange128((int64*)header, desired.region, desired.alignment, (int64*)&expected) == 1)
			break;

	}
	return entry;
}