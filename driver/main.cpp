#include <iostream>
#include <tork.h>
#include <tork/app/OptionStream.h>
using namespace std;

DebugDetectMemoryLeak(global_memory_leak_detection);


void TestOptionStream()
{
    using namespace tork;

    char* args[] = {
        "prog", "-A", "-B", "-h", "-x", "--name=value",
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
        { "x", tork::OptionType::NeedArg },
        { "name", tork::OptionType::Normal },
        { "value", tork::OptionType::MayArg },
        {"id", tork::OptionType::NeedArg},
    };
    try {
        optst.parse(specs);
    }
    catch (tork::InvalidOption& e) {
        cerr << e.what() << endl;
    }
}

int main(int argc, char* argv[])
{
    TestOptionStream();

    return 0;
}
