#include <iostream>

#include <tork/container/Array.h>
#include <tork/memory/shared_ptr.h>
#include <algorithm>
#include <vector>

using std::cout;
using std::endl;
using tork::Array;

void Test_Array_int()
{
	cout << "*** test Array<int> ***" << endl;

	Array<int> a;
	for (int i = 0; i < 10; ++i) {
		a.push_back(i);
	}

	// 全要素出力
	for (auto i : a) {
		cout << i << ' ';
	}
	cout << endl;

	// 逆イテレータ
	for (auto rit = a.rbegin(); rit != a.rend(); ++rit) {
		cout << *rit << ' ';
	}
	cout << endl;

	a.clear();
}

void Test_Array_shared_ptr()
{
	cout << "*** test Array<shared_ptr<int>> ***" << endl;

	using SpInt = tork::shared_ptr<int>;
	Array<SpInt> sa;
	sa.push_back(SpInt::make(20));
	cout << **sa.cbegin() << endl;

	sa.emplace_back(new int(50));
	cout << *sa.back() << endl;

	sa.resize(10, SpInt::make(100));
	for (auto p : sa) {
		cout << *p << ' ';
	}
	cout << endl;
}

void Test_Array()
{
	Test_Array_int();
	Test_Array_shared_ptr();
}
