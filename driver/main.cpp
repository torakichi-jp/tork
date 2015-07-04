#include <iostream>
#include <tork.h>
using namespace std;

DebugDetectMemoryLeak(global_memory_leak_detection);


int main(int argc, char* argv[])
{
    char* args[] = {
        "prog", "-A", "-B", "-h", "-x",
    };
    int len = tork::length_of(args);

    tork::OptionStream optst(len, args);
    try {
        optst.parse("ABCfhxZ");
    }
    catch (tork::InvalidOption& e) {
        cerr << e.what() << endl;
        return 1;
    }

    char ch;
    while (optst >> ch) {
        cout << ch << endl;
    }

    pair<string, tork::OptionHasArg> specs[] = {
        {"name", tork::OptionHasArg::None},
        {"value", tork::OptionHasArg::Required},
        {"id", tork::OptionHasArg::Optional},
    };
    optst.parse(specs);

    return 0;
}
