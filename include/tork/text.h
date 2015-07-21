//******************************************************************************
//
// テキスト処理
//
//******************************************************************************
#ifndef TORK_TEXT_H_INCLUDED
#define TORK_TEXT_H_INCLUDED

#include <string>
#include <sstream>

namespace tork {

struct bad_from_string : std::bad_cast {
    const char* what() const
    {
        return "bad cast from string";
    }
};

struct bad_lexical_cast : std::bad_cast {
    const char* what() const
    {
        return "bad cast";
    }
};

// 文字列に変換
template<class T>
std::string to_string(const T& t)
{
    std::ostringstream os;
    os << t;
    return os.str();
}

// 文字列から変換
template<class T>
T from_string(const std::string& s)
{
    std::istringstream is(s);
    T t;
    if (!(is >> t)) {
        throw bad_from_string();
    }
    return t;
}

// 相互変換
template<class Target, class Source>
Target lexical_cast(Source arg)
{
    std::stringstream interpreter;
    Target result;

    if (!(interpreter << arg)               // arg をストリームに書き込む
      || !(interpreter >> result)           // ストリームから結果を読み取る
      || !(interpreter >> std::ws).eof()) { // ストリームにまだ残っているか
        throw bad_lexical_cast();
    }

    return result;
}


}   // namespace tork

#endif  // TORK_TEXT_H_INCLUDED

