#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>

void HelloThread()
{
    cout << "Hello Thread" << endl;
}

int main()
{
    // 스레드의 생성
    std::thread t(HelloThread);

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
