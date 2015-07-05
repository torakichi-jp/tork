
#include <tork/app/OptionStream.h>
#include <cctype>
#include <iostream>
using namespace std;

namespace tork {

//------------------------------------------------------------------------------
// InvalidOption

InvalidOption::InvalidOption(const string& op)
    :std::logic_error(string("Invalid Option: ") + op),
    name_(op)
{

}

//------------------------------------------------------------------------------
// OptionArgRequired

OptionArgRequired::OptionArgRequired(const string& op)
    :std::logic_error(string("Argument Required: ") + op),
    name_(op)
{

}


//------------------------------------------------------------------------------
// OptionStream

// 空文字列を返す時に使う
const string OptionStream::empty_str_("");

// コンストラクタ
OptionStream::OptionStream(int argc, char* const argv[], int firstOpt)
    :argv_(&argv[firstOpt], &argv[argc]),
    first_arg_index_(argv_.size()),
    is_valid_(false)
{

}

// ストリームクリア
void OptionStream::clear()
{
    first_arg_index_ = argv_.size();
    is_valid_ = false;
    option_list_.clear();
}

// 1文字オプション解析
void OptionStream::parse(const string& optstr)
{
    clear();

    // argv_ の要素を1個ずつ処理
    for (size_t ai = 0; ai < argv_.size(); ++ai) {
        const string& op = argv_[ai];

        // オプション以外の引数かどうかチェック
        if (check_first_arg(ai)) {
            break;
        }

        // 最初のハイフンはチェックしないので1から始める
        for (size_t i = 1; i < op.size(); ++i) {
            // 英数字のみかチェック
            if (!isalnum(op[i])) {
                throw InvalidOption("-" + op);
            }

            // optstrで指定されているかチェック
            size_t pos = optstr.find(op[i]);
            if (pos == string::npos) { // 見つからなかった
                throw InvalidOption("-" + op);
            }
        }

        // オプションリストに追加
        for (size_t i = 1; i < op.size(); ++i) {
            option_list_.push_back( { {op[i]}, ""});
        }
    }

    is_valid_ = true;
}

// オプションじゃない引数かどうかチェック
bool OptionStream::check_first_arg(size_t i)
{
    std::string arg = argv_[i];
    if (arg[0] != '-') {
        // 1文字目がハイフンじゃなかったら、
        // これが最初のオプション以外の引数である
        first_arg_index_ = i;
        return true;
    }
    else if (arg == "--") {
        // "--" と一致したら、この次の引数からが
        // オプション以外の引数である
        first_arg_index_ = i + 1;
        return true;
    }
    return false;
}

// オプション解析
size_t OptionStream::parse_option(size_t i, std::string name, std::string value,
        const OptionSpec& spec)
{
    switch (spec.second) {
    case OptionType::Normal:
        option_list_.push_back({name, ""});
        return i + 1;

    case OptionType::Char:
        option_list_.push_back({name, ""});
        for (auto v : value) {
            option_list_.push_back({{v}, ""});
        }
        return i + 1;

    case OptionType::MayArg:
    case OptionType::NeedArg:
        if (!value.empty()) {
            // value があれば値として追加
            option_list_.push_back({name, value});
            return i + 1;
        }
        if (i + 1 < argv_.size() && argv_[i + 1][0] != '-') {
            // 次の引数を値として追加
            option_list_.push_back({name, argv_[i + 1]});
            return i + 2;
        }

        // 引数にあたるものが見つからなかった

        if (spec.second == OptionType::MayArg) {
            option_list_.push_back({name, ""});
            return i + 1;
        }
        else if (spec.second == OptionType::NeedArg) {
            const string head = name.size() == 1 ? "-" : "--";
            throw OptionArgRequired(head + name);
        }

        break;
    }

    return i + 1;
}

// ストリーム入力演算子
// （1文字オプション用）
OptionStream& operator >>(OptionStream& optStream, char& dest)
{
    if (!optStream.valid()) {
        return optStream;
    }

    if (!optStream.option_list_.empty()) {
        dest = optStream.option_list_.front().name[0];
        optStream.option_list_.pop_front();
    }
    else {
        optStream.is_valid_ = false;
    }

    return optStream;
}

// ストリーム入力演算子
OptionStream& operator >>(OptionStream& optStream, CmdOption& dest)
{
    if (!optStream.valid()) {
        return optStream;
    }

    if (!optStream.option_list_.empty()) {
        dest = optStream.option_list_.front();
        optStream.option_list_.pop_front();
    }
    else {
        optStream.is_valid_ = false;
    }

    return optStream;
}

}   // namespace tork

