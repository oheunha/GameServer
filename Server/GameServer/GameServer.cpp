#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

atomic<bool> ready;
int32 value;

void Producer()
{
    value = 10;

    ready.store(true, memory_order::memory_order_release);
    // ------------------절취선 위로 보장---------------------
}

void Consumer()
{
    // ------------------절취선 아래로 보장---------------------
    while (ready.load(memory_order::memory_order_acquire) == false)
        ;

    cout << value << endl;
}

int main()
{
    ready = false;
    value = 0;
    thread t1(Producer);
    thread t2(Consumer);
    t1.join();
    t2.join();
    
    // Memory Model (정책)
    // 1) Sequentially Consistent (seq_cst) 일관성 있음.
    // 2) Acquire-Release ( acquire, release )
    // 3) Relaxed (relaxed)
        
    // 1) seq_cst (가장 엄격 = 컴파일러 최적화 여지 적음 = 직관적)
    // 가시성 문제 바로 해결! 코드 재배치 바로 해결!
     
    // 2) acquire-release
    // 딱 중간!
    // release 명령 이전의 메모리 명령들이, 해당 명령 이후로 재배치 되는 것을 금지
    // 그리고 acquire로 같은 변수를 읽는 쓰레드가 있다면
    // release 이전의 명령들이 -> acquire 하는 순간에 관찰 가능 ( 가시성 보장 )
    
    // 3) relaxed (자유롭다 = 컴파일러의 최적화 여지 많음 = 직관적이지 않음)
    // 너무나도 자유롭다!
    // 코드 재배치도 멋대로 가능! 가시성 해결 NO!
    // 가장 기본 조건 ( 동일 객체에 대한 동일 관전 순서만 보장 )

    // 인텔, AMD의 경우 애당초 순차적 일관성을 보장을 해서,
    // seq_cst를 써도 별다른 부하가 없음
}
