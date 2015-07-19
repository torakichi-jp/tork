//******************************************************************************
//
// 所有権移動スマートポインタ
//
//******************************************************************************
#ifndef TORK_MEMORY_UNIQUE_PTR_INCLUDED
#define TORK_MEMORY_UNIQUE_PTR_INCLUDED

#include <ostream>
#include "default_deleter.h"

namespace tork {

    namespace impl {

        struct deleter_has_pointer {
            template<class T, class D>
                static typename D::pointer check(typename D::pointer);

            template<class T, class D>
                static T* check(...);
        };

    }   // namespace tork::impl


template<class T, class D = tork::default_deleter<T>>
class unique_ptr {
public:
    typedef T element_type;
    typedef D deleter_type;

    // 戻り値型を decltype で取得
    typedef decltype(impl::deleter_has_pointer::check<T, D>(nullptr)) pointer;

private:
    pointer ptr_ = pointer();
    deleter_type deleter_;

public:

    // デフォルトコンストラクタ
    unique_ptr() : ptr_(pointer()) { }

    // ポインタを受け取るコンストラクタ
    explicit unique_ptr(pointer ptr)
        :ptr_(ptr), deleter_(deleter_type()) { }

    // ポインタと削除子への参照
    unique_ptr(pointer ptr,
            typename std::conditional<std::is_reference<D>::value, D,
                const typename std::remove_reference<D>::type&>::type deleter)
        :ptr_(ptr), deleter_(deleter)
    {

    }

    // ポインタと削除子への右辺値参照
    unique_ptr(pointer ptr,
            typename std::remove_reference<D>::type&& deleter)
        :ptr_(ptr), deleter_(std::move(deleter))
    {
        static_assert(!std::is_reference<D>::value,
            "unique_ptr constructed with reference to rvalue deleter");
    }

    // nullptr
    unique_ptr(nullptr_t) :p_holder_(pointer()) { }

    // ムーブコンストラクタ
    unique_ptr(unique_ptr&& other)
        :ptr_(other.release()), deleter_(std::forward<D>(other.get_deleter()))
    {

    }

    // 別の型からのムーブコンストラクタ
    template<class U, class E,
        class = typename std::enable_if<!std::is_array<U>::value
            && std::is_convertible<typename unique_ptr<U, E>::pointer,
                pointer>::value
            && ((std::is_reference<D>::value && std::is_same<D, E>::value)
                || (!std::is_reference<D>::value
                    && std::is_convertible<E, D>::value)),
            void>::type>
    unique_ptr(unique_ptr<U, E>&& other)
        :ptr_(other.release()), deleter_(std::forward<E>(other.get_deleter()))
    {

    }

    // コピー構築禁止
    unique_ptr(const unique_ptr&) = delete;

    // デストラクタ
    ~unique_ptr()
    {
        if (ptr_) {
            deleter_(ptr_);
        }
    }

    // ムーブ代入
    unique_ptr& operator =(unique_ptr&& other)
    {
        assert(ptr_ != other.ptr_ || ptr_ == pointer());
        reset(other.release());
        deleter_ = std::forward<D>(other.get_deleter());
        return *this;
    }

    // 別の型からのムーブ代入
    template<class U, class E,
        class = typename std::enable_if<
            std::convertible<unique_ptr<U, E>::pointer, pointer>::value
            && !std::is_array<U>::value,
        void>::type>
    unique_ptr& operator =(unique_ptr<U, E>&& other)
    {
        assert(ptr_ != other.get() || ptr_ == pointer());
        reset(other.release());
        deleter_ = std::forward<E>(other.get_deleter());
        return *this;
    }

    // nullptr代入
    unique_ptr& operator =(nullptr_t)
    {
        reset();
        return *this;
    }

    // コピー代入禁止
    unique_ptr& operator =(const unique_ptr&) = delete;

    // 間接参照演算子
    typename std::add_reference<T>::type operator *() const
    {
        return *get();
    }

    // アロー演算子
    pointer operator ->() const
    {
        return get();
    }

    // リソース取得
    pointer get() const { return ptr_; }

    // リソースの所有権放棄
    pointer release()
    {
        pointer pRet = ptr_;
        ptr_ = pointer();
        return pRet;
    }

    // 削除子への参照取得
    deleter_type& get_deleter() { return deleter_; }
    const deleter_type& get_deleter() const { return deleter_; }

    // リセット
    void reset(pointer p = pointer())
    {
        pointer pOld = ptr_;
        ptr_ = p;
        if (pOld) {
            deleter_(pOld);
        }
    }

    // スワップ
    void swap(unique_ptr& other)
    {
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }

    // 有効なリソースを所有しているか
    explicit operator bool() const
    {
        return get() != pointer();
    }

};  // class unique_ptr

//==============================================================================
// 配列版の特殊化
//==============================================================================
template<class T, class D>
class unique_ptr<T[], D> {
public:
    typedef T element_type;
    typedef D deleter_type;

    // 戻り値型を decltype で取得
    typedef decltype(impl::deleter_has_pointer::check<T, D>(nullptr)) pointer;

private:
    pointer ptr_ = pointer();
    deleter_type deleter_;

public:

    // デフォルトコンストラクタ
    unique_ptr() : ptr_(pointer()) { }

    // ポインタを受け取るコンストラクタ
    explicit unique_ptr(pointer ptr)
        :ptr_(ptr), deleter_(deleter_type()) { }

    // ポインタと削除子への参照
    unique_ptr(pointer ptr,
            typename std::conditional<std::is_reference<D>::value, D,
                const typename std::remove_reference<D>::type&>::type deleter)
        :ptr_(ptr), deleter_(deleter)
    {

    }

    // ポインタと削除子への右辺値参照
    unique_ptr(pointer ptr,
            typename std::remove_reference<D>::type&& deleter)
        :ptr_(ptr), deleter_(std::move(deleter))
    {
        static_assert(!std::is_reference<D>::value,
            "unique_ptr constructed with reference to rvalue deleter");
    }

    // nullptr
    unique_ptr(nullptr_t) :p_holder_(pointer()) { }

    // ムーブコンストラクタ
    unique_ptr(unique_ptr&& other)
        :ptr_(other.release()), deleter_(std::forward<D>(other.get_deleter()))
    {

    }

    // コピー構築禁止
    unique_ptr(const unique_ptr&) = delete;

    // デストラクタ
    ~unique_ptr()
    {
        if (ptr_) {
            deleter_(ptr_);
        }
    }

    // ムーブ代入
    unique_ptr& operator =(unique_ptr&& other)
    {
        assert(ptr_ != other.ptr_ || ptr_ == pointer());
        reset(other.release());
        deleter_ = std::forward<D>(other.get_deleter());
        return *this;
    }

    // nullptr代入
    unique_ptr& operator =(nullptr_t)
    {
        reset();
        return *this;
    }

    // コピー代入禁止
    unique_ptr& operator =(const unique_ptr&) = delete;

    // 添え字演算子
    typename std::add_reference<T>::type operator [](size_t i) const
    {
        return get()[i];
    }

    // リソース取得
    pointer get() const { return ptr_; }

    // リソースの所有権放棄
    pointer release()
    {
        pointer pRet = ptr_;
        ptr_ = pointer();
        return pRet;
    }

    // 削除子への参照取得
    deleter_type& get_deleter() { return deleter_; }
    const deleter_type& get_deleter() const { return deleter_; }

    // リセット
    void reset(pointer p = pointer())
    {
        pointer pOld = ptr_;
        ptr_ = p;
        if (pOld) {
            deleter_(pOld);
        }
    }

    // スワップ
    void swap(unique_ptr& other)
    {
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }

    // 有効なリソースを所有しているか
    explicit operator bool() const
    {
        return get() != pointer();
    }

};  // class unique_ptr<T[], D>

// ストリーム出力
template<class charT, class Traits, class T, class D>
std::basic_ostream<charT, Traits>& operator <<(
        std::basic_ostream<charT, Traits>& os, const unique_ptr<T, D>& p)
{
    os << p.get();
    return os;
}

// oeprator ==
template<class T, class D, class U, class E>
bool operator ==(const unique_ptr<T, D>& lhs, const unique_ptr<U, E>& rhs)
{
    return lhs.get() == rhs.get();
}
template<class T, class D>
bool operator ==(const unique_ptr<T, D>& lhs, nullptr_t)
{
    return !lhs;
}
template<class T, class D>
bool operator ==(nullptr_t, const unique_ptr<T, D>& rhs)
{
    return !rhs;
}

// oeprator !=
template<class T, class D, class U, class E>
bool operator !=(const unique_ptr<T, D>& lhs, const unique_ptr<U, E>& rhs)
{
    return lhs.get() != rhs.get();
}
template<class T, class D>
bool operator !=(const unique_ptr<T, D>& lhs, nullptr_t)
{
    return static_cast<bool>(lhs);
}
template<class T, class D>
bool operator !=(nullptr_t, const unique_ptr<T, D>& rhs)
{
    return static_cast<bool>(rhs);
}

// oeprator <
template<class T, class D, class U, class E>
bool operator <(const unique_ptr<T, D>& lhs, const unique_ptr<U, E>& rhs)
{
    return lhs.get() < rhs.get();
}
template<class T, class D>
bool operator <(const unique_ptr<T, D>& lhs, nullptr_t)
{
    return lhs.get() < nullptr;
}
template<class T, class D>
bool operator <(nullptr_t, const unique_ptr<T, D>& rhs)
{
    return nullptr < rhs.get();
}

// oeprator <=
template<class T, class D, class U, class E>
bool operator <=(const unique_ptr<T, D>& lhs, const unique_ptr<U, E>& rhs)
{
    return !(rhs < lhs);
}
template<class T, class D>
bool operator <=(const unique_ptr<T, D>& lhs, nullptr_t)
{
    return !(nullptr < lhs);
}
template<class T, class D>
bool operator <=(nullptr_t, const unique_ptr<T, D>& rhs)
{
    return !(rhs < nullptr);
}

// oeprator >
template<class T, class D, class U, class E>
bool operator >(const unique_ptr<T, D>& lhs, const unique_ptr<U, E>& rhs)
{
    return rhs < lhs;
}
template<class T, class D>
bool operator >(const unique_ptr<T, D>& lhs, nullptr_t)
{
    return nullptr < lhs;
}
template<class T, class D>
bool operator >(nullptr_t, const unique_ptr<T, D>& rhs)
{
    return rhs < nullptr;
}

// oeprator >=
template<class T, class D, class U, class E>
bool operator >=(const unique_ptr<T, D>& lhs, const unique_ptr<U, E>& rhs)
{
    return !(lhs < rhs);
}
template<class T, class D>
bool operator >=(const unique_ptr<T, D>& lhs, nullptr_t)
{
    return !(lhs < nullptr);
}
template<class T, class D>
bool operator >=(nullptr_t, const unique_ptr<T, D>& rhs)
{
    return !(nullptr < rhs);
}

// スワップ
template <class T, class D>
void swap(unique_ptr<T, D>& lhs, unique_ptr<T, D>& rhs)
{
    lhs.swap(rhs);
}


}   // namespace tork

#endif  // TORK_MEMORY_UNIQUE_PTR_INCLUDED

