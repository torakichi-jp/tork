
#include <tork/container/Array.h>
#include <algorithm>
#include <vector>

void Test_Array()
{
    using tork::Array;

    Array<int> a;
    a.reserve(8);
    a.reserve(100);
}
