#include <iostream>
#include <cassert>

#include <tork/memory.h>
#include <tork/debug.h>

using std::cout;
using std::endl;

// �e�X�g�p�̊��N���X
struct B {
    int b = 20;
    virtual ~B() { }
};
// �e�X�g�p�̔h���N���X
struct D : public B {
    int* p;
    explicit D(int n = 100) :p(T_NEW int(n)) { b = n; }
    ~D() { delete p; }  // ������virtual�ɂ��Ă��Ȃ�

    // �R�s�[�֎~
    D(const D&) = delete;
    D& operator =(const D&) = delete;

    // ���[�u�Ń|�C���^�ړ�
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

// weak_ptr �e�X�g
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

    // �z�Q�Ƃ����Ă݂�
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

// shared_ptr �e�X�g
void Test_shared_ptr()
{
    using tork::shared_ptr;

    cout << "*** shared_ptr test ***" << endl;

    shared_ptr<B> sb(new D(5));
    assert(sb->b == 5);
    shared_ptr<void> sb2 = sb;
    shared_ptr<void> sb3(std::move(sb2));
    assert(sb3.use_count() == 2);

    shared_ptr<void> sp(new D[20], tork::default_deleter<D[]>());
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

// default_deleter �e�X�g
void Test_default_deleter()
{
    B* p = new D[10];
    tork::default_deleter<B[]> da;
    da(p);

    tork::default_deleter<B> db;
    db(new D);

    tork::default_deleter<D> dd;
    tork::default_deleter<B> db2 = dd;
    //tork::default_deleter<int> di = dd;
}