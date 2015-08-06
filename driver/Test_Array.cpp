#include <iostream>

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

} // annonymous namespace

void Test_Array()
{
	//Test_Array_int();
	//Test_Array_shared_ptr();

	Test_SharedArrayObject();
}
