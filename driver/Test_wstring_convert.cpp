#include <iostream>

#include <string>
#include <locale>

void Test_wstring_convert()
{
    std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>
        cvt(new std::codecvt_byname<wchar_t, char, std::mbstate_t>("ja-JP"));
    std::string str = cvt.to_bytes(L"テスト");
    std::wstring wstr = cvt.from_bytes("テスト");

    std::cout << str << std::endl;
}
