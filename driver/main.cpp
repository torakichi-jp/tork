#include <iostream>
#include <tork.h>
#include <tork/memory/shared_ptr.h>
#include <memory>
//using namespace std;

DebugDetectMemoryLeak(global_memory_leak_detection);

void Test_OptionStream();    // OptionStream テスト
void Test_optional();        // optional テスト
void Test_default_deleter(); // default_deleter テスト


// エントリポイント
int main(int argc, char* argv[])
{
    /*
    Test_OptionStream();
    Test_optional();
    Test_default_deleter();
    */

    struct B {
        int b = 20;
        virtual ~B() { }
    };
    struct D : public B {
        int* p = new int(100);
        ~D() {
            delete p;
        }
    };
    tork::shared_ptr<void> p(new D);
    tork::shared_ptr<B> sb(new D);
    std::cout << sb->b << std::endl;
    tork::shared_ptr<void> sp(new D[20], tork::default_deleter<D[]>());
    sp.reset();
    tork::shared_ptr<void> sb2 = sb;
    tork::shared_ptr<void> sb3(std::move(sb2));
    std::cout << sb3.use_count() << std::endl;
    tork::shared_ptr<int> pi(nullptr);

    return 0;
}

// default_deleter テスト
void Test_default_deleter()
{
    struct B {
        int b = 20;
        virtual ~B() { }
    };
    struct D : public B {
        int* p = new int(100);
        ~D() {
            delete p;
        }
    };

    B* p = new D[10];
    tork::default_deleter<B[]> da;
    da(p);

    tork::default_deleter<B> db;
    db(new D);

    tork::default_deleter<D> dd;
    tork::default_deleter<B> db2 = dd;
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

