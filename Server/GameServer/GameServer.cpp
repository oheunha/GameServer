#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>
#include "CoreMacro.h"
#include "ThreadManager.h"
#include "RefCounting.h"

using KnightRef = TSharedPtr<class Knight>;
using InventoryRef = TSharedPtr<class Inventory>;

class Knight : public RefCountable
{
public:
	Knight()
	{
		cout << "Knight()" << endl;
	}

	~Knight()
	{
		cout << "~Knight()" << endl;
	}
};

int main()
{
	// 1) 이미 만들어진 클래스 대상으로 사용 불가
	// 2) 순환 (Cycle) 문제
	
	

	// shared_ptr
	// weak_ptr
	
	// [Knight | RefCountingBlock(uses, weak)]
	
	// [T*][RefCountBlock*]

	// RefCountBlock(useCount(shared(0)), weakCount(2))
	shared_ptr<Knight> spr = make_shared<Knight>();
	weak_ptr<Knight> wpr = spr; // 수명주기엔 영향안줌

	bool expired = wpr.expired(); // Knight 아직 유효하니?
	shared_ptr<Knight> spr2 = wpr.lock(); // weak포인터가 shared포인터로 변신
	if (spr2 != nullptr)
	{

	}



	// unique_ptr : 알아서 삭제 / 복사 막아둠
	unique_ptr<Knight> k2 = make_unique<Knight>();
	unique_ptr<Knight> k3 = std::move(k2);

	// 예제1)
	//KnightRef k1(new Knight());
	//k1->ReleaseRef();
	//KnightRef k2(new Knight());
	//k2->ReleaseRef();
	//
	//k1->SetTarget(k2);
	//k2->SetTarget(k1);
	//
	//k2->SetTarget(nullptr);
	//k1->SetTarget(nullptr);
	//
	//k1 = nullptr;
	//k2 = nullptr;
}
