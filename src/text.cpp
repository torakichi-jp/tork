
#include <tork/text.h>
#include <string>
#include <locale>

using namespace std;
using Converter = wstring_convert<codecvt<wchar_t, char, mbstate_t>>;
using ConvByName = codecvt_byname<wchar_t, char, mbstate_t>;

namespace tork {

// ワイド文字へ変換
wstring ToWide(const string& str)
{
    Converter conv(new ConvByName(""));
    return conv.from_bytes(str);
}
wstring ToWide(const char* str)
{
    Converter conv(new ConvByName(""));
    return conv.from_bytes(str);
}

// ワイド文字から変換
string FromWide(const wstring& wstr)
{
    Converter conv(new ConvByName(""));
    return conv.to_bytes(wstr);
}
string FromWide(const wchar_t* wstr)
{
    Converter conv(new ConvByName(""));
    return conv.to_bytes(wstr);
}


}   // namespace tork
