#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>
atomic<bool> flag;

int main()
{
    flag = false;

    //flag = true;
    flag.store(true, memory_order::memory_order_seq_cst); // default

    //bool val = flag;
    bool val = flag.load(memory_order::memory_order_seq_cst); // default

    // 이전 flag 값을 prev에 넣고, flag 값을 수정
    {
        bool prev = flag.exchange(true);
        //bool prev = flag;
        //flag = true;
    }

    // CAS (Compare-And-Swap) 조건부 수정
    {
        bool expected = false;
        bool desired = true;
        flag.compare_exchange_strong(expected, desired);

        // Spurious Failure
        if (flag == expected)
        {
            // 다른 쓰레드의 interreption을 받아서 중간에 실패할 수 있음
            // if (묘한 상황)
            //  return false;


            //expected = flag;
            flag = desired;
            return true;
        }
        else
        {
            expected = flag;
            return false;
        }

        while (true)
        {
            bool expected = false;
            bool desired = true;
            flag.compare_exchange_weak(expected, desired);
        }

    }
    
}
