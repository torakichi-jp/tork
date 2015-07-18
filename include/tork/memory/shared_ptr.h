//******************************************************************************
//
// 参照カウンタ方式資源共有スマートポインタ
//
//******************************************************************************

#ifndef TORK_MEMORY_SHARED_PTR_H_INCLUDED
#define TORK_MEMORY_SHARED_PTR_H_INCLUDED

#include <type_traits>
#include <typeinfo>
#include <cassert>
#include <ostream>
#include <utility>
#include "default_deleter.h"
#include "allocator.h"
#include "../define.h"

#include "ptr_holder.h"

namespace tork {

// 前方宣言
template<class T>
    class weak_ptr;

//==============================================================================
// 参照カウンタ式スマートポインタ
//==============================================================================
template <class T>
class shared_ptr {
    impl::ptr_holder_base* p_holder_ = nullptr;

    // T じゃない型のにアクセスできるように friend 宣言
    template<class> friend class shared_ptr;
    template<class> friend class weak_ptr;

public:
    typedef T element_type; // 要素型

    //--------------------------------------------------------------------------
    // コンストラクタ

    // デフォルトコンストラクタ
    shared_ptr() :p_holder_(nullptr) { }

    // ポインタ設定
    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    explicit shared_ptr(U* ptr)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<U, default_deleter<U>, tork::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, default_deleter<U>(), tork::allocator<void>());
    }

    // ポインタとカスタム削除子設定
    template<class U, class Deleter,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    shared_ptr(U* ptr, Deleter deleter)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<U, Deleter, tork::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, deleter, tork::allocator<void>());
    }

    // ポインタ、カスタム削除子、アロケータ設定
    template<class U, class Deleter, class Alloc,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    shared_ptr(U* ptr, Deleter deleter, Alloc alloc)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<U, Deleter, Alloc>;

        p_holder_ = Holder::create_holder(ptr, deleter, alloc);
    }

    // nullptr
    explicit shared_ptr(nullptr_t) :p_holder_(nullptr) { }

    // コピーコンストラクタ
    shared_ptr(const shared_ptr& other)
        :p_holder_(other.p_holder_)
    {
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }

    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    shared_ptr(const shared_ptr<U>& other)
        :p_holder_(other.p_holder_)
    {
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }

    // ムーブコンストラクタ
    shared_ptr(shared_ptr&& other)
        :p_holder_(other.p_holder_)
    {
        other.p_holder_ = nullptr;
    }

    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    shared_ptr(shared_ptr<U>&& other)
        :p_holder_(other.p_holder_)
    {
        other.p_holder_ = nullptr;
    }

    // weak_ptr から生成
    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    explicit shared_ptr(const weak_ptr<U>& other)
        :p_holder_(other.p_holder_)
    {
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }


    // デストラクタ
    ~shared_ptr()
    {
        if (p_holder_) {
            p_holder_->release();
        }
    }

    // コピー代入演算子
    shared_ptr& operator =(const shared_ptr& other)
    {
        if (other.p_holder_ == this->p_holder_) return *this;
        shared_ptr(other).swap(*this);
        return *this;
    }

    template<class U>
    shared_ptr& operator =(const shared_ptr<U>& other)
    {
        if (other.p_holder_ == this->p_holder_) return *this;
        shared_ptr(other).swap(*this);
        return *this;
    }

    // ムーブ代入演算子
    shared_ptr& operator =(shared_ptr&& other)
    {
        assert(other.p_holder_ != this->p_holder_);
        shared_ptr(std::move(other)).swap(*this);
        return *this;
    }

    template<class U>
    shared_ptr& operator =(shared_ptr<U>&& other)
    {
        assert(other.p_holder_ != this->p_holder_);
        shared_ptr(std::move(other)).swap(*this);
        return *this;
    }

    // ポインタ取得
    T* get() const {
        return p_holder_ ? static_cast<T*>(p_holder_->get()) : nullptr;
    }

    // 関節参照演算子
    auto operator *() -> typename std::add_reference<T>::type
    {
        return *get();
    }
    auto operator *() const -> const typename std::add_reference<T>::type
    {
        return *get();
    }

    // アロー演算子
    T*       operator ->() { return get(); }
    const T* operator ->() const { return get(); }

    // 入れ替え
    void swap(shared_ptr& other)
    {
        impl::ptr_holder_base* pTmp = this->p_holder_;
        this->p_holder_ = other.p_holder_;
        other.p_holder_ = pTmp;
    }

    // 再設定
    void reset()
    {
        shared_ptr().swap(*this);
    }

    template<class U>
    void reset(U* ptr)
    {
        shared_ptr(ptr).swap(*this);
    }

    template<class U, class Deleter>
    void reset(U* ptr, Deleter deleter)
    {
        shared_ptr(ptr, deleter).swap(*this);
    }

    template<class U, class Deleter, class Alloc>
    void reset(U* ptr, Deleter deleter, Alloc alloc)
    {
        shared_ptr(ptr, deleter, alloc).swap(*this);
    }

    // 参照カウンタ取得
    int use_count() const {
        return p_holder_ ? p_holder_->get_ref_counter() : 0;
    }

    // 削除子取得
    template<class D>
    D* get_deleter() const
    {
        return static_cast<D*>(p_holder_->get_deleter(typeid(D)));
    }

    // 所有権を持っているのが自分だけかどうか
    bool unique() const { return use_count() == 1; }

    // 有効なポインタかどうか（nullptr でないか）
    explicit operator bool() const { return get() != nullptr; }

    // shared_ptr 作成
    template<class... Args>
    static shared_ptr<T> make(Args&&... args);

    template<class Alloc, class... Args>
    static shared_ptr<T> make_allocate(Alloc alloc, Args&&... args);


    template<class T1, class T2>
    friend shared_ptr<T1> static_pointer_cast(const shared_ptr<T2>& r);

    template<class T1, class T2>
    friend shared_ptr<T1> const_pointer_cast(const shared_ptr<T2>& r);

    template<class T1, class T2>
    friend shared_ptr<T1> dynamic_pointer_cast(const shared_ptr<T2>& r);

};  // class shared_ptr

//==============================================================================
// shared_ptr の配列形式の特殊化
//==============================================================================
template <class T>
class shared_ptr<T[]> {
    impl::ptr_holder_base* p_holder_ = nullptr;

    template<class> friend class shared_ptr;
    template<class> friend class weak_ptr;

public:
    typedef T element_type; // 要素型

    //--------------------------------------------------------------------------
    // コンストラクタ

    // デフォルトコンストラクタ
    shared_ptr() :p_holder_(nullptr) { }

    // ポインタ設定
    explicit shared_ptr(T* ptr)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<T, default_deleter<T[]>, tork::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, default_deleter<T[]>(), tork::allocator<void>());
    }

    // ポインタとカスタム削除子設定
    template<class Deleter>
    shared_ptr(T* ptr, Deleter deleter)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<T, Deleter, tork::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, deleter, tork::allocator<void>());
    }

    // ポインタ、カスタム削除子、アロケータ設定
    template<class Deleter, class Alloc>
    shared_ptr(T* ptr, Deleter deleter, Alloc alloc)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<T, Deleter, Alloc>;

        p_holder_ = Holder::create_holder(ptr, deleter, alloc);
    }

    // nullptr
    explicit shared_ptr(nullptr_t) :p_holder_(nullptr) { }

    // コピーコンストラクタ
    shared_ptr(const shared_ptr& other)
        :p_holder_(other.p_holder_)
    {
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }

    // ムーブコンストラクタ
    shared_ptr(shared_ptr&& other)
        :p_holder_(other.p_holder_)
    {
        other.p_holder_ = nullptr;
    }

    // weak_ptr から生成
    shared_ptr(const weak_ptr<T[]>& other)
        :p_holder_(other.p_holder_)
    {
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }


    // デストラクタ
    ~shared_ptr()
    {
        if (p_holder_) {
            p_holder_->release();
        }
    }

    // コピー代入演算子
    shared_ptr& operator =(const shared_ptr& other)
    {
        if (other.p_holder_ == this->p_holder_) return *this;
        shared_ptr(other).swap(*this);
        return *this;
    }

    // ムーブ代入演算子
    shared_ptr& operator =(shared_ptr&& other)
    {
        assert(other.p_holder_ != this->p_holder_);
        shared_ptr(std::move(other)).swap(*this);
        return *this;
    }

    // ポインタ取得
    T* get() const {
        return p_holder_ ? static_cast<T*>(p_holder_->get()) : nullptr;
    }

    // 配列アクセス
    auto operator [](size_t i) -> typename std::add_reference<T>::type
    {
        return get()[i];
    }
    auto operator [](size_t i) const -> const typename std::add_reference<T>::type
    {
        return get()[i];
    }

    // 入れ替え
    void swap(shared_ptr& other)
    {
        impl::ptr_holder_base* pTmp = this->p_holder_;
        this->p_holder_ = other.p_holder_;
        other.p_holder_ = pTmp;
    }

    // 再設定
    void reset()
    {
        shared_ptr().swap(*this);
    }

    void reset(T* ptr)
    {
        shared_ptr(ptr).swap(*this);
    }

    template<class Deleter>
    void reset(T* ptr, Deleter deleter)
    {
        shared_ptr(ptr, deleter).swap(*this);
    }

    template<class Deleter, class Alloc>
    void reset(T* ptr, Deleter deleter, Alloc alloc)
    {
        shared_ptr(ptr, deleter, alloc).swap(*this);
    }

    // 参照カウンタ取得
    int use_count() const {
        return p_holder_ ? p_holder_->get_ref_counter() : 0;
    }

    // 削除子取得
    template<class D>
    D* get_deleter() const
    {
        return static_cast<D*>(p_holder_->get_deleter(typeid(D)));
    }

    // 所有権を持っているのが自分だけかどうか
    bool unique() const { return use_count() == 1; }

    // 有効なポインタかどうか（nullptr でないか）
    explicit operator bool() const { return get() != nullptr; }


    template<class T1, class T2>
    friend shared_ptr<T1> static_pointer_cast(const shared_ptr<T2>& r);

    template<class T1, class T2>
    friend shared_ptr<T1> const_pointer_cast(const shared_ptr<T2>& r);

    template<class T1, class T2>
    friend shared_ptr<T1> dynamic_pointer_cast(const shared_ptr<T2>& r);

};  // class shared_ptr<T[]>


// 削除子取得
template<class D, class T>
D* get_deleter(const shared_ptr<T>& p)
{
    return p.get_deleter<D>();
}

// スタティックキャスト
template<class T, class U>
shared_ptr<T> static_pointer_cast(const shared_ptr<U>& r)
{
    if (r.p_holder_ == nullptr) {
        return shared_ptr<T>();
    }
    static_cast<T*>(r.get());
    shared_ptr<T> sptr;
    sptr.p_holder_ = r.p_holder_;
    r.p_holder_->add_ref();
    return sptr;
}

// const キャスト
template<class T, class U>
shared_ptr<T> const_pointer_cast(const shared_ptr<U>& r)
{
    if (r.p_holder_ == nullptr) {
        return shared_ptr<T>();
    }
    const_cast<T*>(r.get());
    shared_ptr<T> sptr;
    sptr.p_holder_ = r.p_holder_;
    r.p_holder_->add_ref();
    return sptr;
}

// ダイナミックキャスト
template<class T, class U>
shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& r)
{
    if (r.p_holder_ == nullptr) {
        return shared_ptr<T>();
    }
    if (dynamic_cast<T*>(r.get())) {
        shared_ptr<T> sptr;
        sptr.p_holder_ = r.p_holder_;
        r.p_holder_->add_ref();
        return sptr;
    }
    else {
        return shared_ptr<T>();
    }
}


// operator ==
template<class T, class U>
bool operator ==(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)
{
    return lhs.get() == rhs.get();
}
template<class T>
bool operator ==(const shared_ptr<T>& lhs, nullptr_t)
{
    return lhs.get() == nullptr;
}
template<class T>
bool operator ==(nullptr_t, const shared_ptr<T>& rhs)
{
    return nullptr == rhs.get();
}

// operator !=
template<class T, class U>
bool operator !=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)
{
    return lhs.get() != rhs.get();
}
template<class T>
bool operator !=(const shared_ptr<T>& lhs, nullptr_t)
{
    return lhs.get() != nullptr;
}
template<class T>
bool operator !=(nullptr_t, const shared_ptr<T>& rhs)
{
    return nullptr != rhs.get();
}

// operator <
template<class T, class U>
bool operator <(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)
{
    return lhs.get() < rhs.get();
}
template<class T>
bool operator <(const shared_ptr<T>& lhs, nullptr_t)
{
    return lhs.get() < nullptr;
}
template<class T>
bool operator <(nullptr_t, const shared_ptr<T>& rhs)
{
    return nullptr < rhs.get();
}

// operator <=
template<class T, class U>
bool operator <=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)
{
    return !(rhs < lhs);
}
template<class T>
bool operator <=(const shared_ptr<T>& lhs, nullptr_t)
{
    return !(nullptr < lhs);
}
template<class T>
bool operator <=(nullptr_t, const shared_ptr<T>& rhs)
{
    return !(rhs < nullptr);
}

// operator >
template<class T, class U>
bool operator >(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)
{
    return rhs < lhs;
}
template<class T>
bool operator >(const shared_ptr<T>& lhs, nullptr_t)
{
    return nullptr < lhs;
}
template<class T>
bool operator >(nullptr_t, const shared_ptr<T>& rhs)
{
    return rhs < nullptr;
}

// operator >=
template<class T, class U>
bool operator >=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)
{
    return !(lhs < rhs);
}
template<class T>
bool operator >=(const shared_ptr<T>& lhs, nullptr_t)
{
    return !(lhs < nullptr);
}
template<class T>
bool operator >=(nullptr_t, const shared_ptr<T>& rhs)
{
    return !(nullptr < rhs);
}

// ストリーム出力
template<class charT, class Traits, class T>
std::basic_ostream<charT, Traits>& operator <<(
        std::basic_ostream<charT, Traits>& os, const shared_ptr<T>& p)
{
    os << p.get();
    return os;
}

// swap()
template<class T>
void swap(const shared_ptr<T>& lhs, const shared_ptr<T>& rhs)
{
    lhs.swap(rhs);
}



// 効率的な shared_ptr の作成
template<class T> template<class... Args>
shared_ptr<T> shared_ptr<T>::make(Args&&... args)
{
    using Alloc = tork::allocator<void>;
    auto p = impl::ptr_holder_alloc<T, Alloc>::create_holder(
            Alloc(), std::forward<Args>(args)...);
    shared_ptr<T> sptr;
    sptr.p_holder_ = p;
    return sptr;
}

// アロケータ指定版
template<class T> template<class Alloc, class... Args>
shared_ptr<T> shared_ptr<T>::make_allocate(Alloc alloc, Args&&... args)
{
    auto p = impl::ptr_holder_alloc<T, Alloc>::create_holder(
            alloc, std::forward<Args>(args)...);
    shared_ptr<T> sptr;
    sptr.p_holder_ = p;
    return sptr;
}

// 非メンバ版
template<class T, class... Args>
shared_ptr<T> make_shared(Args&&... args)
{
    return shared_ptr<T>::make(std::forward<Args>(args)...);
}

// アロケータ指定版の非メンバ版
template<class T, class Alloc, class... Args>
shared_ptr<T> allocate_shared(Alloc alloc, Args&&... args)
{
    return shared_ptr<T>::make_allocate(alloc, std::forward<Args>(args)...);
}


// 前方宣言
template<class T>
class enable_shared_from_this;
namespace impl {
    template<class T1>
    void do_enable_shared(
            enable_shared_from_this<T1>* pEs,
            impl::ptr_holder_base* pHolder);
}

//==============================================================================
// enable_shared_from_this
//==============================================================================
template<class T>
class enable_shared_from_this {
private:
    weak_ptr<T> weak_this_;
protected:
    enable_shared_from_this() : weak_this_() { }
    enable_shared_from_this(enable_shared_from_this const &) { }
    enable_shared_from_this& operator=(enable_shared_from_this const &) { return *this; }
    ~enable_shared_from_this() { }
public:
    shared_ptr<T> shared_from_this() { return shared_ptr<T>(weak_this_); }
    shared_ptr<T const> shared_from_this() const { return shared_ptr<T const>(weak_this_); }

    template<class T1>
    friend void impl::do_enable_shared(
            enable_shared_from_this<T1>* pEs,
            impl::ptr_holder_base* pHolder);
};

namespace impl {

    template<class T>
    void do_enable_shared(
            enable_shared_from_this<T>* pEs,
            impl::ptr_holder_base* pHolder)
    {
        pEs->weak_this_.p_holder_ = pHolder;
        pEs->weak_this_.p_holder_->add_weak_ref();
    }

    inline void do_enable_shared(const volatile void*, const volatile void*)
    {
        // do nothing
    }

}

}   // namespace tork


// ハッシュの前方宣言
template<class T> struct std::hash;

// ハッシュの shared_ptr の特殊化
template<class T>
struct std::hash<tork::shared_ptr<T>> {

    typedef size_t result_type;
    typedef tork::shared_ptr<T> argument_type;

    // ハッシュ関数
    result_type operator ()(const argument_type& keyval)
    {
        return std::hash<T*>()(keyval.get());
    }

};  // struct std::hash<shared_ptr<T>>


#endif  // TORK_MEMORY_SHARED_PTR_H_INCLUDED

