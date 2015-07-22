#include <iostream>
#include <fstream>

#include <string>
#include <locale>
#include <codecvt>

#include <tork/text.h>

#include <Windows.h>


// UTF-8をchar32_tに変換して文字数を数える
size_t u8strlen(const std::string& s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s).size();
}

void Test_wstring_convert()
{
    MessageBox(nullptr, tork::ToWide("ほげほげ文字列").c_str(), nullptr, MB_OK);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    std::string str = cvt.to_bytes(L"テストABC");
    std::wstring wstr = cvt.from_bytes(str);

    size_t s = u8strlen(str);
    std::cout << s << std::endl;
}


