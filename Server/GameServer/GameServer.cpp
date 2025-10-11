#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>
#include "CoreMacro.h"
#include "ThreadManager.h"
#include "RefCounting.h"
#include "Memory.h"
#include "Allocator.h"
#include "Container.h"
#include "LockFreeStack.h"

DECLSPEC_ALIGN(16)
class Data
{
public:
	SListEntry _entry;

	int32 _rand = rand() % 1000;
};

SListHeader* GHeader;
int main()
{
	GHeader = new SListHeader();
	ASSERT_CRASH(((uint64)GHeader % 16) == 0);
	InitializeHead(GHeader);
	
	for (int32 i = 0; i < 3; ++i)
	{
		GThreadManager->Launch([]()
			{
				while (true)
				{
					Data* data = new Data();
					ASSERT_CRASH(((uint64)GHeader % 16) == 0);

					PushEntrySList(GHeader, (SListEntry*)data);
					this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			});
	}

	for (int32 i = 0; i < 2; ++i)
	{
		GThreadManager->Launch([]()
			{
				while (true)
				{
					Data* pop = nullptr;
					pop = (Data*)PopEntrySList(GHeader);

					if (pop)
					{
						cout << pop->_rand << endl;
						delete pop;
					}
					else
					{
						cout << "None" <<  endl;
					}
				}
			});
	}

}