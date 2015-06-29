#include <iostream>
#include <tork.h>
using namespace std;

DebugDetectMemoryLeak(global_memory_leak_detection);


int main()
{
    tork::optional<int> opint = 60;
    cout << opint << endl;
    cout << sizeof(opint) << endl;

    DebugPrint("テスト\n");

    return 0;
}
