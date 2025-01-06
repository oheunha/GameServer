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

#include <vector>
#include <thread>

// 소수 구하기
bool isPrime(int number)
{
	if (number <= 1)
		return false;
	if (number == 2 || number == 3)
		return true;

	for (int i = 2; i < number; i++)
	{
		if ((number % i) == 0)
			return false;
	}
	return true;
}

int CoountPrime(int start, int end)
{
	int count = 0;

	for (int number = start; number <= end; number++)
	{
		if (isPrime(number))
			count++;
	}
	return count;
}

// 1과 자기 자신으로만 나뉘면 그것을 소수라고 함
// 2,3,5,7,11

int main()
{
	const int MAX_NUMBER = 1000'0;

	// 1000 = 168
	// 10'000 = 1229
	// 1'000'000 = 78498

	// 1~MAX_NUMBER까지의 소수 개수
	vector<thread> threads;

	int coreCount = thread::hardware_concurrency(); // 코어갯수 실시간 병렬실행할 수 있는 개수
	int jobCount = (MAX_NUMBER / coreCount) + 1;

	atomic<int> primeCount = 0; // 결과물

	for (int i = 0; i < coreCount; i++)
	{
		int start = (i * jobCount) + 1;
		int end = min(MAX_NUMBER, ((i + 1) * jobCount));

		threads.push_back(thread([start, end, &primeCount]()
			{
				primeCount += CoountPrime(start, end);
			}));
	}

	for (thread& t : threads)
		t.join();

	cout << primeCount << endl;
}
