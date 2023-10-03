#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>
#include "ConcurrentQueue.h"
#include "ConcurrentStack.h"

LockQueue<int32> q;
LockFreeStack<int32> s;

void Push()
{ 
    while (true)
    {
        int32 value = rand() % 100;
        s.Push(value);

        this_thread::sleep_for(1ms);
    }
}

void Pop()
{
    while (true)
    {
        auto data = s.TryPop();
        if (nullptr != data)
        {
            cout << *data << endl;
        }
    }
}


int main()
{
    // 락프리 확인 체크
    //shared_ptr<int32> ptr;
    //bool value = std::atomic_is_lock_free(&ptr);
    //cout << value << endl;

    thread t1(Push);
    thread t2(Pop);
    thread t3(Pop);
    
    
    t1.join();
    t2.join();
    t3.join();

}
