﻿#include <iostream>
#include <cassert>
#include <algorithm>
#include <memory>

#include <tork/memory.h>
#include <tork/debug.h>

using std::cout;
using std::endl;

// テスト用の基底クラス
struct B {
    int b = 20;

    // あえてvirtualデストラクタにしていない
    ~B() { }

    // dynamic_cast ができるようにvirtual関数定義
    virtual void dummy() { }
};
// テスト用の派生クラス
struct D : public B {
    int* p;

    explicit D(int n = 100) :p(T_NEW int(n)) { b = n; }
    
    ~D() { delete p; }

    // コピー禁止
    D(const D&) = delete;
    D& operator =(const D&) = delete;

    // ムーブでポインタ移動
    D(D&& other) :B(std::move(other))
    {
        p = other.p;
        other.p = nullptr;
    }
    D& operator =(D&& other)
    {
        B::operator =(std::move(other));
        p = other.p;
        other.p = nullptr;
    }
};

template<class Ptr>
void Show_pointer_traits()
{
    using traits = std::pointer_traits<Ptr>;
    cout << "*** " << typeid(traits).name() << " ***" << endl;

    cout << "pointer         " << typeid(traits::pointer).name() << endl;
    cout << "element_type    " << typeid(traits::element_type).name() << endl;
    cout << "difference_type " << typeid(traits::difference_type).name() << endl;
    cout << "rebind<float>   " << typeid(traits::rebind<float>).name() << endl;
}

void Test_pointer_traits()
{
    Show_pointer_traits<tork::shared_ptr<int>>();
    Show_pointer_traits<tork::weak_ptr<size_t>>();
    Show_pointer_traits<tork::unique_ptr<double[]>>();
}

// enable_shared_from_this テスト
void Test_enable_shared_from_this()
{
    struct X : public tork::enable_shared_from_this<X> {
        tork::shared_ptr<X> f()
        {
            // thisを指すshared_ptrオブジェクトを作る
            return shared_from_this();
        }
    };

    auto p = tork::make_shared<X>();
    auto q = p->f();

    assert(p == q);
}

// unique_ptr テスト
void Test_unique_ptr()
{
    using tork::unique_ptr;

    cout << "*** unique_ptr test ***" << endl;

    unique_ptr<int> emp;

    unique_ptr<int> ui(new int(1234));
    unique_ptr<int> ui2(std::move(ui));
    unique_ptr<int> ui3;
    ui3 = std::move(ui2);
    cout << *ui3 << endl;
    cout << ui2 << ", " << ui3 << endl;

    unique_ptr<D> ud(new D);
    unique_ptr<B> ub(std::move(ud));
    delete dynamic_cast<D*>(ub.release());

    unique_ptr<B> ub2;
    unique_ptr<D> ud2(new D(987));
    ub2 = std::move(ud2);
    ud2.reset(dynamic_cast<D*>(ub2.release()));
    cout << *ud2->p << endl;

    const int num = 20;
    unique_ptr<int[]> uia(new int[num]);
    for (int i = 0; i < num; ++i) {
        uia[i] = i;
        cout << uia[i];
        if (i < num - 1) cout << ' ';
        else cout << endl;
    }

    unique_ptr<int> a(new int(80));
    unique_ptr<int> b(new int(160));
    cout << std::boolalpha;
    cout << (a == a) << ' ' << (a != b) << endl;
    cout << (a != nullptr) << ' ' << (nullptr != a) << endl;
    cout << (a < b) << ' ' << (a > b) << endl;
    cout << (a <= b) << ' ' << (a >= b) << endl;
    cout << (a < nullptr) << ' ' << (nullptr < a) << endl;
    cout << (a > nullptr) << ' ' << (nullptr > a) << endl;
    cout << (a <= nullptr) << ' ' << (nullptr <= a) << endl;
    cout << (a >= nullptr) << ' ' << (nullptr >= a) << endl;

    // make_unique() テスト
    {
        auto ip = tork::make_unique<int>(456); // 通常版
        cout << *ip << endl;

        const int N = 10;
        auto ap = tork::make_unique<int[]>(N); // 配列版
        auto b = &ap[0]; // begin
        auto e = &ap[N]; // end
        int i = 0;
        std::for_each(b, e, [&i](int &n) { n = i++; });
        std::for_each(b, e, [](int n) { cout << n << ' '; });
        cout << endl;
    }
}

// weak_ptr テスト
void Test_weak_ptr()
{
    using tork::shared_ptr;
    using tork::weak_ptr;

    cout << "*** weak_ptr test ***" << endl;

    shared_ptr<B> sb(tork::make_shared<D>(123));
    weak_ptr<B> wb;
    wb = sb;
    cout << wb.lock()->b << endl;
    sb.reset();
    cout << wb.lock() << endl;

    // 循環参照させてみる
    struct CB;
    struct CA {
        weak_ptr<CB> p;
        int n = 50;
    };
    struct CB {
        weak_ptr<CA> p;
        int n = 100;
    };
    shared_ptr<CA> ca(new CA);
    shared_ptr<CB> cb(new CB);
    ca->p = cb;
    cb->p = ca;
    cout << ca->p.lock()->n << ' ' << cb->p.lock()->n << endl;
}

// shared_ptr テスト
void Test_shared_ptr()
{
    using tork::shared_ptr;

    cout << "*** shared_ptr test ***" << endl;

    shared_ptr<B> sb(new D(5));
    assert(sb->b == 5);
    shared_ptr<void> sb2 = sb;
    shared_ptr<void> sb3(std::move(sb2));
    assert(sb3.use_count() == 2);

    shared_ptr<void> sp(T_NEW D[20], tork::default_deleter<D[]>());
    auto del = tork::get_deleter<tork::default_deleter<D[]>>(sp);
    std::cout << "deleter : " << del << std::endl;
    sp.reset();

    shared_ptr<int> pi(nullptr);
    shared_ptr<int> pi2(new int(50));
    pi = std::move(pi2);
    assert(*pi == 50);
    assert(pi2 == nullptr);

    shared_ptr<const D> sc(new D);
    shared_ptr<B> sd = tork::const_pointer_cast<D>(sc);
    cout << "sd.use_count : " << sd.use_count() << endl;
    cout << "sd->b : " << sd->b << endl;
    assert(sd == tork::dynamic_pointer_cast<D>(sd));

    tork::shared_ptr<B> sbp(T_NEW B);
    assert(nullptr == tork::dynamic_pointer_cast<D>(sbp));

    tork::shared_ptr<int[]> ai(new int[20]);
    ai[5] = 400;
    assert(ai[5] == 400);
    cout << "deleter : "
        << tork::get_deleter<tork::default_deleter<int[]>>(ai)
        << endl;
    shared_ptr<void> sv;
    sv = ai;
    cout << "sv[5] : " << tork::static_pointer_cast<int[]>(sv)[5] << endl;

    shared_ptr<B> b;
    shared_ptr<D> d(new D);
    std::cout << std::boolalpha;
    std::cout << (b == b) << ' ' << (b == nullptr) << std::endl;
    std::cout << (b != d) << ' ' << (d != nullptr) << std::endl;
    std::cout << (b < d) << ' ' << (b > d) << std::endl;
    std::cout << (b <= d) << ' ' << (b >= d) << std::endl;

    shared_ptr<int> n(new int(10));
    shared_ptr<int> m(new int(20));
    std::swap(n, m);
    assert(*n == 20 && *m == 10);

    auto mp = shared_ptr<D>::make(987);
    shared_ptr<B> mpb;
    mpb = mp;
    std::cout << *tork::dynamic_pointer_cast<D>(mpb)->p << std::endl;

    auto alp = tork::allocate_shared<D>(std::allocator<D>(), 1234);
    std::cout << *alp->p << std::endl;

    std::hash<shared_ptr<int>> h;
    auto hp = shared_ptr<int>(T_NEW int(150));
    std::cout << "hash : " << h(hp) << std::endl;
}

// default_deleter テスト
void Test_default_deleter()
{
    D* p = new D[10];
    tork::default_deleter<D[]> da;
    da(p);

    //tork::default_deleter<B> db;
    //db(new D);

    tork::default_deleter<D> dd;
    tork::default_deleter<B> db2 = dd;
    //tork::default_deleter<int> di = dd;
}