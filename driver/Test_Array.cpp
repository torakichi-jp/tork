﻿#include <iostream>

#include <tork/container/Array.h>
#include <tork/memory/shared_ptr.h>
#include <algorithm>
#include <vector>

using std::cout;
using std::endl;
using tork::Array;

namespace {
	template<class T>
	void print(const Array<T>& a)
	{
		cout << "{ ";
		for (auto v : a) {
			cout << v << ' ';
		}
		cout << "}" << endl;
	}
}

void Test_Array_int()
{
	cout << "*** test Array<int> ***" << endl;

	Array<int> a;
	for (int i = 0; i < 10; ++i) {
		a.push_back(i);
	}

	// 全要素出力
	print(a);

	// 逆イテレータ
	for (auto rit = a.rbegin(); rit != a.rend(); ++rit) {
		cout << *rit << ' ';
	}
	cout << endl;

	for (size_t i = 0; i < a.size(); ++i) {
		a[i] *= 10;
		cout << a.at(i) << ' ';
	}
	cout << endl;

	Array<int> b;
	a.swap(b);
	a.clear();

	Array<int> a2(3, 999);
	a2.emplace_back(20);
	a2.emplace_back(32);
	print(a2);

	Array<int> a3(b.cbegin(), b.cend());
	a3.pop_back();
	a3.pop_back();
	print(a3);

	Array<int> a4 = a3;
	print(a4);

	Array<int> a5 = std::move(a4);
	print(a5);

	Array<int> a6{ 5, 6, 7, 8 };
	print(a6);
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
