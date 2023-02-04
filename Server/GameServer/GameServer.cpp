#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>

void HelloThread()
{
    cout << "Hello Thread" << endl;
}

void HelloThread_2(int32 num)
{
    cout << "[2]" << num << endl;
}

int main()
{
    // 스레드의 생성
    std::thread t(HelloThread);

    vector<std::thread> v;
    for (int32 i = 0; i < 10; ++i)
    {
        v.push_back(std::thread(HelloThread_2, i));
    }

    for (int32 i = 0; i < 10; ++i)
    {
        if (v[i].joinable())
        {
            v[i].join();
        }
    }

    cout << t.joinable() << endl;
    
    cout << "Hello Main" << endl;

    cout << t.hardware_concurrency() << endl; // 동시 스레드 수

    cout << t.joinable() << endl;

    cout << t.get_id() << endl; // 스레드 아이디

    if (t.joinable())
    {
        t.join();
    }

    //t.detach();
  
}
