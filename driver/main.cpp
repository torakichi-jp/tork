#include <iostream>
#include <tork.h>
#include <tork/memory/weak_ptr.h>
#include <tork/memory/allocator.h>
#include <memory>

// メモリリーク検出
DebugDetectMemoryLeak(global_memory_leak_detection);

void stopper()
{
    std::cout << "enter to continue." << std::flush;
    std::cin.clear();
    std::cin.get();
}

void Test_OptionStream();    // OptionStream テスト
void Test_optional();        // optional テスト
void Test_default_deleter(); // default_deleter テスト
void Test_shared_ptr();      // shared_ptr テスト
void Test_weak_ptr();        // weak_ptr テスト

// テスト用の基底クラス
struct B {
    int b = 20;
    virtual ~B() { }
};
// テスト用の派生クラス
struct D : public B {
    int* p;
    explicit D(int n = 100) :p(T_NEW int(n)) { b = n; }
    ~D() { delete p; }
    D(const D&) = delete;
    D& operator =(const D&) = delete;

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

// エントリポイント
int main()
{
    /*
    Test_OptionStream();
    Test_optional();
    Test_default_deleter();
    Test_shared_ptr();
    */
    Test_weak_ptr();

    stopper();
    return 0;
}

// weak_ptr テスト
void Test_weak_ptr()
{
    using tork::shared_ptr;
    using tork::weak_ptr;
    using std::cout;
    using std::endl;

    shared_ptr<B> sb(tork::make_shared<D>(123));
    weak_ptr<B> wb;
    wb = sb;
    cout << wb.lock()->b << endl;
    sb.reset();
    cout << wb.lock() << endl;

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

    shared_ptr<B> sb(new D);
    std::cout << sb->b << std::endl;
    shared_ptr<void> sb2 = sb;
    shared_ptr<void> sb3(std::move(sb2));
    std::cout << sb3.use_count() << std::endl;

    tork::shared_ptr<void> sp(new D[20], tork::default_deleter<D[]>());
    auto del = tork::get_deleter<tork::default_deleter<D[]>>(sp);
    std::cout << "deleter : " << del << std::endl;
    sp.reset();

    tork::shared_ptr<int> pi(nullptr);
    tork::shared_ptr<int> pi2(new int(50));
    pi = std::move(pi2);
    DebugPrint("%d, %d\n", *pi, pi2.use_count());

    tork::shared_ptr<const D> sc(new D);
    tork::shared_ptr<B> sd = tork::const_pointer_cast<D>(sc);
    DebugPrint("%d, %d\n", sd.use_count(), sd->b);

    tork::shared_ptr<B> sbp(T_NEW B);
    DebugPrint("dynamic_pointer_cast : %p\n", tork::dynamic_pointer_cast<D>(sbp).get());

    tork::shared_ptr<int[]> ai(new int[20]);
    ai[5] = 400;
    DebugPrint("%d\n", ai[5]);
    std::cout << "deleter : " << tork::get_deleter<tork::default_deleter<int[]>>(ai)
        << std::endl;
    shared_ptr<void> sv;
    sv = ai;
    DebugPrint("%d\n", tork::static_pointer_cast<int[]>(sv)[5]);

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
    DebugPrint("%d, %d\n", *n, *m);

    std::cout << n << ' ' << m << std::endl;

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
    B* p = new D[10];
    tork::default_deleter<B[]> da;
    da(p);

    tork::default_deleter<B> db;
    db(new D);

    tork::default_deleter<D> dd;
    tork::default_deleter<B> db2 = dd;
    //tork::default_deleter<int> di = dd;
}

// optional テスト
void Test_optional()
{
    using tork::optional;
    using namespace std;

    cout << "optional test" << endl;

    optional<int> oi = 60;
    if (oi.valid()) {
        cout << "valid :" << oi << endl;
    }
    else {
        cout << "invalid." << endl;
    }
}

// OptionStream テスト
void Test_OptionStream()
{
    using namespace tork;
    using namespace std;

    cout << "OptionStream test" << endl;

    char* args[] = {
        "prog", "-ABCZ", "-h", "-x", "20",
        "--name:hoge", "--id", "500",
    };
    int len = length_of(args);

    tork::OptionStream optst(len, args);

    try {
        optst.parse("ABCfhxZ");
    }
    catch (tork::InvalidOption& e) {
        cerr << e.what() << endl;
 //       return;
    }

    char ch;
    while (optst >> ch) {
        cout << ch << endl;
    }

    tork::OptionSpec specs[] = {
        { "ABCfhZ", tork::OptionType::Char },
        { "x", tork::OptionType::MayArg},
        { "name", tork::OptionType::NeedArg },
        { "value", tork::OptionType::Normal },
        {"id", tork::OptionType::NeedArg},
    };
    try {
        optst.parse(specs);
    }
    catch (tork::InvalidOption& e) {
        cerr << e.what() << endl;
    }
    catch (tork::OptionArgRequired& e) {
        cerr << e.what() << endl;
    }

    tork::CmdOption opt;
    while (optst >> opt) {
        cout << opt.name << " : \"" << opt.arg << "\"" << endl;
    }
}

