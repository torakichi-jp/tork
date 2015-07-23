
#include <tork/container/Array.h>

void Test_Array()
{
    using tork::Array;

    Array<int> a;
    a.reserve(8);
    a.reserve(100);
}
