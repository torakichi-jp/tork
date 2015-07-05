#include <iostream>
#include <tork.h>
#include <tork/app/OptionStream.h>
using namespace std;

DebugDetectMemoryLeak(global_memory_leak_detection);

void TestOptionStream();    // OptionStream テスト

// エントリポイント
int main(int argc, char* argv[])
{
    TestOptionStream();

    return 0;
}

// OptionStream テスト
void TestOptionStream()
{
    using namespace tork;

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

