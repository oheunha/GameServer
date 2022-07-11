#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

//_declspec(thread) int32 value; 예전의 TLS사용 
thread_local int32 LThreadId = 0; // 일반 변수가 아닌 TLS 영역으로 잡힘
thread_local queue<int32> q;

void ThreadMain(int32 threadId)
{
	LThreadId = threadId;
	while (true)
	{
		cout << "Hi! I am Thread " << LThreadId << endl;
		this_thread::sleep_for(1s);
	}
}

int main()
{
	vector<thread> threads;

	for (int32 i = 0; i < 10; ++i)
	{
		int32 threadId = i + 1;
		threads.push_back(thread(ThreadMain, threadId));
	}

	for (thread& t : threads)
		t.join();
  
}
