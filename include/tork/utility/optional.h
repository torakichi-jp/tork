//******************************************************************************
// オプション変数テンプレート
// C++11以上
// 関数が成功したかどうかを、戻り値として使って表すことを想定
//------------------------------------------------------------------------------
// *使い方*
//
//  optional<Data> data = DoSomething();
//  if (data.valid()) {
//      // なんか処理
//      data->Do();
//      Function(*data);
//  }
//
//******************************************************************************

#ifndef TORK_OPTIONAL_H_INCLUDED
#define TORK_OPTIONAL_H_INCLUDED

#include <ostream>
#include <cassert>
#include <type_traits>

namespace tork {

// 空オプション型
struct optional_empty { };

// オプション変数テンプレート
template<class T>
class optional {
public:
    // コンストラクタ
    optional() : m_valid(false) { }
    optional(const optional& other): m_valid(false) {
        if (other) {
            construct(other.get());
        }
    }
    optional(const T& t): m_valid(false) { construct(t); }
    optional(const optional_empty&): m_valid(false) { }

    // デストラクタ
    ~optional() { if (m_valid) destruct(); }

    // コピー代入
    optional& operator =(const optional& other) {
        optional& self = *this;
        if (self && other) {
            self.get() = other.get();
        }
        else if (!self && other) {
            construct(other.get());
        }
        else if (self && !other) {
            clear();
        }
        else if (!self && !other) {
            // どーせ空なので何もする必要はない
        }
        return *this;
    }

    optional& operator =(const T& t) {
        if (m_valid) {
            get() = t;
        }
        else {
            construct(t);
        }
        return *this;
    }

    optional& operator =(const optional_empty&) {
        if (m_valid) {
            destruct();
        }
        return *this;
    }

    // アクセサ
    T&       get()       { assert(m_valid); return *get_as_ptr(); }
    const T& get() const { assert(m_valid); return *get_as_ptr(); }
    T*       ptr()       { return &get(); }
    const T* ptr() const { return &get(); }
    T&       operator *()        { return get(); }
    const T& operator *() const  { return get(); }
    T*       operator ->()       { return &get(); }
    const T* operator ->() const { return &get(); }

    // 有効かどうか調べる
    bool valid() const { return m_valid; }
    bool invalid() const { return !m_valid; }
    explicit operator bool() const { return m_valid; }

    // 無効にする
    void clear() { if (m_valid) destruct(); }

private:
    typename std::aligned_storage<
        sizeof(T),
        std::alignment_of<T>::value
    >::type m_data;     // 格納するメモリ領域
    bool m_valid;       // 有効かどうか

    // 保持データへのポインタ取得
    T* get_as_ptr() {
        void* p = &m_data;
        return static_cast<T*>(p);
    }

    const T* get_as_ptr() const {
        const void* p = &m_data;
        return static_cast<const T*>(p);
    }

    // データ構築
    void construct(const T& t) {
        if (new(get_as_ptr()) T(t)) {
            m_valid = true;
        }
    }

    // データ破棄
    void destruct() {
        get_as_ptr()->~T();
        m_valid = false;
    }

};  // class optional


// 標準ストリーム出力演算子
template<class charT, class Traits, class T>
std::basic_ostream<charT, Traits>&
    operator <<(std::basic_ostream<charT, Traits>& ost, const optional<T>& o)
{
    if (o) {
        ost << *o;
    }
    return ost;
}


}   // namespace tork

#endif  // TORK_OPTIONAL_H_INCLUDED

