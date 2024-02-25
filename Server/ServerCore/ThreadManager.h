#pragma once

#include <thread>
#include <functional>

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(function<void(void)> callback);
	void Join();

	static void InitTLS(); // TLS영역 초기화
	static void DestroyTLS();

private:
	Mutex _lock;
	vector<thread> _threads;

};

