#include <iostream>
#include <tork/optional.h>

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
