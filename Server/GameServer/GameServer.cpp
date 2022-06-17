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
        while (_locked)
        {

        }

        _locked = true;
    }

    void unlock()
    {
        _locked = false;

    }

private:
    volatile bool _locked = false;


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
