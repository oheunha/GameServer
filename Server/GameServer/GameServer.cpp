#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>

mutex m;
queue<int32> q; // 공용 데이터 관리
HANDLE handle;
void Producer() // 데이터 생성
{
    while (true)
    {
        {
            unique_lock<mutex> lock(m);
            q.push(100);
        }

        ::SetEvent(handle); // Signaled 상태로 세팅

        this_thread::sleep_for(10000ms);
    }
}

void Consumer() // 데이터를 소모
{
    while (true)
    {
        ::WaitForSingleObject(handle, INFINITE); // 이벤트가 Non-Signaled면 이 스레드는 잠들게 되며 ( 대기쿨쿨 ), Signaled면 아래 코드를 동작합니다.
        // ::ResetEvent(handle); 수동이면 직접 신호를 꺼주어야한다.

        unique_lock<mutex> lock(m);
        if (q.empty() == false)
        {
            int32 data = q.front();
            q.pop();
            cout << data << endl;
        }
    }
}



int main( )
{
    // 데이터가 있을 때만 알려주세요.
    // 커널 오브젝트
    // Usage Count
    // Signal (파란불) / Non-Signal (빨간불) << bool
    // Auto / Manual << bool
    handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*bManualReset*/, FALSE/*bInitialState*/, NULL);



    thread t1(Producer);
    thread t2(Consumer);
    
    t1.join();
    t2.join();

    ::CloseHandle(handle);
}
