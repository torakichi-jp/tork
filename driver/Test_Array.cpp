#include <iostream>

#include <tork/container/Array.h>
#include <algorithm>
#include <vector>

void Test_Array()
{
    using tork::Array;

    Array<int> a;
	for (int i = 0; i < 10; ++i) {
		a.push_back(i);
	}

	// 全要素出力
	for (auto i : a) {
		std::cout << i << ' ';
	}
	std::cout << std::endl;

	// 逆イテレータ
	for (auto rit = a.rbegin(); rit != a.rend(); ++rit) {
		std::cout << *rit << ' ';
	}
	std::cout << std::endl;
}
