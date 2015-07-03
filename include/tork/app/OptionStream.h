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

// オプションに引数があるかどうかの指定用
//                        無し  必要      どちらでも
enum class OptionHasArg { None, Required, Optional };

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
    template<template<class T, class Allocator = std::allocator<T>> class Container>
    void parse(const Container<std::pair<std::string, OptionHasArg>>& specs);

    template<size_t S>
    void parse(const std::pair<std::string, OptionHasArg> (&specs)[S]);

    bool valid() const { return is_valid_; }

    // ストリームから読み込めるかどうか
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


    // 詳細オプション解析
    template<class T>
    void parse_(const T& specs, int num);
    void parse_(const std::pair<std::string, OptionHasArg>& spec);

};  // class OptionStream


// 詳細オプション解析
template<template<class T, class Allocator = std::allocator<T>> class Container>
inline void OptionStream::parse(const Container<std::pair<std::string, OptionHasArg>>& specs)
{
    parse_(specs, specs.size());
}

template<size_t S>
inline void OptionStream::parse(const std::pair<std::string, OptionHasArg> (&specs)[S])
{
    parse_(specs, S);
}

template<class T>
void OptionStream::parse_(const T& specs, int num)
{
    for (auto v : specs) {
        parse_(v);
    }
}

}   // namespace tork

#endif  // OPTION_STREAM_H_INCLUDED

