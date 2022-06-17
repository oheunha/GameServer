#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

class SpinLock
{
public:
    void lock()
    { 
        // CAS ( Compare-And-Swap )

        bool expected = false;
        bool desired = true;

        // CAS 의사 코드
        /*
        if (_locked == expected)
        {
            expected = _locked;
            _locked = desired;
            return true;
        }
        else
        {
            expected = _locked;
            return false;
        }
        */
        
        while (false == _locked.compare_exchange_strong(expected, desired)) // atomic 한 함수 
        {
            expected = false;
        }
        
    }

    void unlock()
    {
        //_locked = false;
        _locked.store(false);

    }

private:
    atomic<bool> _locked = false;


};

SpinLock spinLock;
int32 sum = 0;
mutex m;

void Add()
{
    for (int32 i = 0; i < 10'0000; i++)
    {
        lock_guard<SpinLock>guard(spinLock);
        sum++;
    }
}

void Sub()
{
    for (int32 i = 0; i < 10'0000; i++)
    {
        lock_guard<SpinLock>guard(spinLock);
        sum--;
    }
}

int main()
{
    thread t1(Add);
    thread t2(Sub);

    t1.join();
    t2.join();


    cout << sum << endl;

}
