#ifndef OPTION_STREAM_H_INCLUDED
#define OPTION_STREAM_H_INCLUDED

// parse() を呼ぶまでストリームは無効状態
// parse() を呼んで、成功するとストリームが有効状態になり、>> が使えるようになる
// ストリームが空の状態で >> を呼ぶと無効状態になり、再度 parse() を呼ぶまで
// >> は使えない

#include <string>
#include <vector>
#include <list>
#include <stdexcept>
#include <algorithm>
#include <regex>
#include "../debug.h"

namespace tork {

// 無効なオプション例外
class InvalidOption : public std::logic_error {
    std::string name_;
public:
    InvalidOption(const std::string& op);
    const std::string& name() const { return name_; }
};

// オプション構造体
struct CmdOption {
    std::string name;   // オプション名
    std::string arg;    // 引数値（なければ空文字）
};

// オプションタイプ
enum class OptionType { Normal, NeedArg, MayArg, Char };

using OptionSpec = std::pair<std::string, OptionType>;

// コマンドラインオプションの処理
class OptionStream {
public:

    // コンストラクタ
    // firstOpt: オプションの最初のインデックス
    // システムから渡される argv[0] はプログラム名なので、デフォルトは1にした
    // prog cmd -op みたいな場合は2を指定すれば良い
    OptionStream(int argc, char* const argv[], int firstOpt = 1);

    // 引数取得
    const std::string& get_arg(size_t i) const
    {
        if (i < argv_.size()) {
            return argv_[i];
        }
        else {
            return empty_str_;
        }
    }

    // 1文字オプション解析
    // optstr: 1文字オプションの列としての文字列を指定する
    // 英数字以外は無視、大文字小文字は区別
    void parse(const std::string& optstr);

    // 詳細オプション解析
    // STLコンテナ版
    template<template<class T, class Allocator = std::allocator<T>> class Container>
    void parse(const Container<OptionSpec>& specs);

    // 配列版
    template<size_t S>
    void parse(const OptionSpec (&specs)[S]);

    // ポインタ＋サイズ版
    void parse(const OptionSpec* pSpecs, int num);

    // ストリームから読み込めるかどうか
    bool valid() const { return is_valid_; }
    explicit operator bool() const { return valid(); }


    // ストリームからオプション取得
    friend OptionStream& operator >>(OptionStream& optStream, char& dest);
    friend OptionStream& operator >>(OptionStream& optStream, CmdOption& dest);

private:
    std::vector<std::string> argv_; // オプション文字列リスト
    int first_arg_index_;           // オプション以外の引数の最初のインデックス

    std::list<CmdOption> option_list_;    // オプションリスト

    bool is_valid_;     // ストリームが有効かどうか

    static const std::string empty_str_; // 空文字列を返す時に使う


    // オプション以外の引数かどうかチェック
    // i: argv_ のインデックス
    bool check_first_arg(size_t i);

    // 詳細オプション解析
    template<class Iter>
    void parse_details(Iter first, Iter last);

    // name をspecから検索
    template<class Iter>
    Iter find_spec(Iter first, Iter last, const std::string& name) const;

    // 1文字オプションをspecから検索
    template<class Iter>
    Iter find_char(Iter first, Iter last, const std::string& ch) const;

    // 長いオプション
    size_t parse_long_option(size_t i, std::string name, std::string value,
        const OptionSpec& spec);

    // 1文字オプション
    size_t parse_char_option(size_t i, std::string opt, std::string rest,
        const OptionSpec& spec);

};  // class OptionStream


// 詳細オプション解析
template<template<class T, class Allocator = std::allocator<T>> class Container>
inline void OptionStream::parse(const Container<OptionSpec>& specs)
{
    parse_details(specs.begin(), specs.end());
}

template<size_t S>
inline void OptionStream::parse(const OptionSpec (&specs)[S])
{
    parse_details(&specs[0], &specs[S]);
}

inline void OptionStream::parse(const OptionSpec* pSpecs, int num)
{
    parse_details(&pSpecs[0], &pSpecs[num]);
}

// 詳細オプション解析実装
// specs: [0, num) のコンテナor配列 要素型はOptionSpec
template<class Iter>
void OptionStream::parse_details(Iter first, Iter last)
{
    for (size_t i = 0; i < argv_.size(); ) {
        // オプション以外の引数かどうかチェック
        if (check_first_arg(i)) {
            break;
        }

        std::regex re;
        std::smatch m;

        // 長いオプション
        re = "--(\\w+)([:=](\\w+))?";
        if (regex_match(argv_[i], m, re)) {
            auto& spec = *find_spec(first, last, m[1].str());
            i = parse_long_option(i, m[1].str(), m[3].str(), spec);
            continue;
        }
        // 1文字オプション
        re = "-(\\w)(\\w+)?";
        if (regex_match(argv_[i], m, re)) {
            try {
                auto& spec = *find_char(first, last, m[1].str());
                i = parse_char_option(i, m[1].str(), m[2].str(), spec);
            }
            catch (InvalidOption&) {
                auto& spec = *find_spec(first, last, m[1].str());
                i = parse_char_option(i, m[1].str(), m[2].str(), spec);
            }
            continue;
        }

        throw InvalidOption(argv_[i]);
    }
}

template<class Iter>
Iter OptionStream::find_spec(Iter first, Iter last, const std::string& name) const
{
    auto it = std::find_if(
            first, last,
            [&name](const OptionSpec& spec) -> bool {
                return name == spec.first;
            });
    if (it == last) {
        throw InvalidOption(name);
    }
    return it;
}

template<class Iter>
Iter OptionStream::find_char(Iter first, Iter last, const std::string& ch) const
{
    // オプションタイプが1文字オプションのspec を探す
    auto it = std::find_if(
            first, last,
            [](const OptionSpec& spec) -> bool {
                return OptionType::Char == spec.second;
            });
    if (it == last) {
        throw InvalidOption(ch);
    }
    if (it->first.find(ch[0]) == std::string::npos) {
        throw InvalidOption(ch);
    }
    return it;
}

}   // namespace tork

#endif  // OPTION_STREAM_H_INCLUDED

