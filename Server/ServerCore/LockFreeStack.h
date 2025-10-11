#pragma once

/*
	1차 시도
*/
/*
template<typename T>
struct Node
{
	T data;
	Node* node;
};

struct SListEntry
{
	SListEntry* next;
};

class Data //: public SListEntry
{
public:
	SListEntry _entry;
	int32 _hp;
	int32 _mp;
};

struct SListHeader
{
	SListEntry* next = nullptr;
};

// 
// [][][]
// [Header]
// 

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntrySList(SListHeader* header);
*/

/*
	2차 시도
*/
/*
struct SListEntry
{
	SListEntry* next;
};

class Data //: public SListEntry
{
public:
	SListEntry _entry;
	int32 _hp;
	int32 _mp;
};

struct SListHeader
{
	SListEntry* next = nullptr;
};

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntrySList(SListHeader* header);
*/

/*
	3차 시도
*/
DECLSPEC_ALIGN(16) // 메모리 16바이트 정렬 힌트
struct SListEntry
{
	SListEntry* next;
};

DECLSPEC_ALIGN(16)
struct SListHeader
{
	SListHeader()
	{
		alignment = 0;
		region = 0;
	}

	union
	{
		struct
		{
			uint64 alignment;
			uint64 region;
		} DUMMYSTRUCTNAME;
	
		struct
		{
			// alignment 구성요소두개로 접근가능
			uint64 depth : 16;
			uint64 sequence : 48;

			// region 구성요소두개로 접근가능
			uint64 reserved : 4;
			uint64 next : 60;
		} HeaderX64;
	};

	SListEntry* next = nullptr;
};

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntrySList(SListHeader* header);