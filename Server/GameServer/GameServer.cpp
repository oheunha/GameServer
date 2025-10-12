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

class Knight
{
public:
	int32 _hp = rand() % 1000;
};

int main()
{
	for (int32 i = 0; i < 2; ++i)
	{
		GThreadManager->Launch([]()
			{
				while (true)
				{
					Knight* knight = xnew <Knight>();
					cout << knight->_hp << endl;
					this_thread::sleep_for(std::chrono::milliseconds(10));
					xdelete(knight);
				}
			});
	}

	GThreadManager->Join();
}