#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

// atomic : All-Or-Nothing
atomic<int32> sum = 0;

// DB
//
// A라는 유저 인벤에서 집행검을빼고
// B라는 유저 인벤에 집행검 추가


void Add()
{
    for (int32 i = 0; i < 100'0000; i++)
    {
        sum.fetch_add(1);
        //sum++;
        /*
            int32 =  eax = sum;
            eax = eax + 1;
            sum = eax;
        */
    }
}

void Sub()
{
    for (int32 i = 0; i < 100'0000; i++)
    {
        sum.fetch_sub(1);
        //sum--;
        /*
          int32 =  eax = sum;
          eax = eax - 1;
          sum = eax;
      */
    }
}

int main()
{
    //Add();
    //Sub();
    //
    std::thread t1(Add);
    std::thread t2(Sub);

    t1.join();
    t2.join();
    cout << sum << endl;

}
