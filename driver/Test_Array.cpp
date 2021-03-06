﻿#include <iostream>

#include <tork/container/Array.h>
#include <tork/container/SharedArray.h>
#include <tork/memory/shared_ptr.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iterator>

using std::cout;
using std::endl;
using tork::Array;
using tork::SharedArray;

namespace {

template<class C>
void print(const C& a)
{
	cout << "{ ";
	for (auto v : a) {
		cout << v << ' ';
	}
	cout << "}" << endl;
}

template<class T, class A, template<class, class> class C>
void print(const C<std::shared_ptr<T>, A>& a)
{
	cout << "{ ";
	for (auto v : a) {
		cout << *v << ' ';
	}
	cout << "}" << endl;
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
	a6.reserve(100);
	a6.shrink_to_fit();

	a5 = a6;
	print(a5);

	Array<int> a7{ 10, 11, 12, 13, 14 };
	a5 = std::move(a7);
	print(a5);

	a5 = { 55, 66, 77, 88 };
	print(a5);

	a5.assign(3, 999);
	print(a5);

	// 入力イテレータによる構築など
	{
		std::stringstream ss;
		ss << "0 1 2 3 4 5 6 7 8 9";
		std::istream_iterator<int> it(ss);
		std::istream_iterator<int> last;
		Array<int> v(it, last);
		print(v);

		ss.clear();
		ss << "10 20 30 40 50 60 70 80 90 100";
		Array<int> v2;
		v2.assign(it, last);
		print(v2);
	}
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

void Test_SharedArrayObject()
{
	using Obj = tork::impl::SharedArrayObject<int, std::allocator<int>>;
	Obj* pObj = Obj::create(std::allocator<int>(), 10);
	pObj->expand(20);
	pObj->add(20);
	pObj->add(30);
	pObj->pop_back();
	pObj->resize(20, 999);
	pObj->resize(5, 100);
	for (size_t i = 0; i < pObj->size; ++i) {
		cout << pObj->p_data[i] << endl;
	}
	pObj->clear();
	Obj::destroy(pObj);
}

void Test_SharedArray()
{
	using IntArray = SharedArray<int>;

	IntArray a;
	a.push_back(10);
	print(a);

	IntArray a2(5, 20);
	print(a2);

	std::vector<int> v{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	IntArray a3(v.begin(), v.end());
	print(a3);

	{
		std::stringstream ss;
		ss << "0 10 20 30 40 50 60 70 80 90";
		std::istream_iterator<int> it(ss);
		std::istream_iterator<int> last;
		IntArray ia(it, last);
		print(ia);
	}

	IntArray a4 = a3;
	print(a4);

	IntArray a5 = std::move(a4);
	print(a5);

	IntArray a6{ 10, 9, 8, 7 };
	print(a6);

	a4 = a5;
	print(a4);

	a4 = std::move(a6);
	print(a4);

	a4 = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
	print(a4);

	{
		tork::SharedArray<std::shared_ptr<int>> a(10);
		for (int i = 0; i < 10; ++i) {
			a[i] = std::make_shared<int>(i);
		}
		a.erase(a.begin() + 2);
		print(a);
		a.erase(a.begin() + 2, a.end() - 2);
		print(a);
		a.reserve(20);
	}

	IntArray a7;
	a7.reserve(100);
	a7 = { 0, 1, 2, 3 };
	a7.shrink_to_fit();

	a7.insert(a7.begin() + 2, 99);
	print(a7);

	a7 = { 0, 1, 2, 3 };
	a7.insert(a7.begin() + 2, 4, 999);
	print(a7);

	a7 = { 0, 1, 2, 3 };
	v = { 10, 9, 8, 7 };
	a7.insert(a7.begin() + 2, v.begin(), v.end());
	print(a7);

	{
		std::stringstream ss;
		ss << "10 20 30 40 50 60 70 80 90";
		std::istream_iterator<int> it(ss);
		std::istream_iterator<int> last;
		IntArray ia = { 0, 1, 2, 3 };
		ia.insert(ia.begin() + 2, it, last);
		print(ia);
	}

	a7 = { 0, 1, 2, 3 };
	a7.insert(a7.begin() + 2, { 9, 8, 7, 6 });
	print(a7);

	IntArray a8{ 9, 10, 11, 12, 13 };
	swap(a7, a8);
	print(a7);
	print(a8);

	IntArray x{ 0, 1, 2, 3 };
	IntArray y{ 0, 1, 2, 3 };
	IntArray z{ 1, 2, 3 };
	bool b;
	b = (x == y);
	b = (x != z);
	b = (y < z);
	b = (x >= z);
}

} // anonymous namespace

void Test_Array()
{
	//Test_Array_int();
	//Test_Array_shared_ptr();

	//Test_SharedArrayObject();

	Test_SharedArray();
}
