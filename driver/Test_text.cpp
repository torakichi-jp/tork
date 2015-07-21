#include <iostream>
#include <string>
#include <tork/text.h>
using std::cout;
using std::endl;

void Test_text()
{
    cout << "*** text.h test ***" << endl;

    std::string str = tork::to_string(123);
    cout << str << endl;

    int n = tork::from_string<int>("987");
    cout << n << endl;

    n = tork::lexical_cast<int>("1111");
    str = tork::lexical_cast<std::string>(5678);
    cout << n << endl;
    cout << str << endl;
}

