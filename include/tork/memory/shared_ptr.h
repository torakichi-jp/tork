//******************************************************************************
//
// 参照カウンタ方式資源共有スマートポインタ
//
//******************************************************************************

#ifndef TORK_MEMORY_SHARED_PTR_H_INCLUDED
#define TORK_MEMORY_SHARED_PTR_H_INCLUDED

#include <memory>
#include <type_traits>
#include <typeinfo>
#include <cassert>
#include <ostream>
#include <utility>
#include "default_deleter.h"
#include "allocator.h"
#include "../define.h"

namespace tork {

// 前方宣言
template<class T>
    class shared_ptr;
template<class T>
    class weak_ptr;

namespace impl {

    //==========================================================================
    // ポインタホルダ基底クラス
    //==========================================================================
    class ptr_holder_base {
        int ref_counter_ = 0;   // 参照カウンタ
        int weak_counter_ = 0;  // ウィークカウンタ

    protected:
        void* ptr_ = nullptr;   // 保持するポインタ

    public:
        ptr_holder_base(void* ptr) : ptr_(ptr) { }
        virtual ~ptr_holder_base() { }

        // 削除子取得
        virtual void* get_deleter(const std::type_info&) const
        {
            return nullptr;
        }

        // ポインタ取得
        void* get() { return ptr_; }

        // 各カウンタ取得
        int get_ref_counter() const { return ref_counter_; }
        int get_weak_counter() const { return weak_counter_; } 


        // 参照カウンタ増
        void add_ref()
        {
            ++ref_counter_;
            add_weak_ref();
        }

        // 参照カウンタ減
        // 0になったらリソース削除
        void release()
        {
            --ref_counter_;
            assert(ref_counter_ >= 0);
            if (ref_counter_ == 0) {
                destroy();
            }
            release_weak_ref();
        }

        // ウィークカウンタ増
        void add_weak_ref()
        {
            ++weak_counter_;
        }

        // ウィークカウンタ減
        // 0になったらホルダ削除
        void release_weak_ref()
        {
            --weak_counter_;
            assert(weak_counter_ >= 0);
            if (weak_counter_ == 0) {
                destroy_holder();
            }
        }

    private:
        virtual void destroy() = 0;         // リソース削除
        virtual void destroy_holder() = 0;  // ホルダ自身を削除

    };  // class ptr_holder_base

    //==========================================================================
    // ポインタホルダ
    //==========================================================================
    template<class T, class Deleter, class Alloc>
    class ptr_holder : public ptr_holder_base {
        Deleter deleter_;   // 削除子
        Alloc alloc_;       // アロケータ
    public:

        void destroy() override { deleter_(static_cast<T*>(ptr_)); }  // リソース削除

        // 削除子取得
        void* get_deleter(const type_info& tid) const override
        {
            return (tid == typeid(Deleter))
                ? const_cast<void*>(static_cast<const void*>(&deleter_)) :
                  nullptr;
        }

        // ホルダ作成
        static ptr_holder* create_holder(T* ptr, Deleter deleter, Alloc alloc)
        {
            // リソースがなければホルダも作らない
            if (ptr == nullptr) {
                return nullptr;
            }

            // 継承を利用して Alloc::construct() が ptr_holder の
            // private コンストラクタにアクセスできるようにするためのクラス
            struct holder_impl : ptr_holder<T, Deleter, Alloc> {
                holder_impl(T* p, Deleter d, Alloc a)
                    : ptr_holder(p, d, a) { }
            };

            // アロケータの再束縛
            using Holder = holder_impl;
            //using Holder = ptr_holder<T, Deleter, Alloc>;
            using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
            using Traits = std::allocator_traits<Allocator>;
            Allocator a = alloc;

            holder_impl* p = Traits::allocate(a, 1);
            if (p == nullptr) {
                // ホルダの領域を確保できなかったら、リークを防ぐため
                // リソースを解放しておく
                deleter(ptr);
                return nullptr;
            }

            Traits::construct(a, p, ptr, deleter, alloc);
            return p;
        }

        // ホルダ破棄（自殺するので注意して扱うこと）
        void destroy_holder() override
        {
            // アロケータの再束縛
            using Holder = ptr_holder<T, Deleter, Alloc>;
            using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
            using Traits = std::allocator_traits<Allocator>;
            Allocator a = alloc_;

            Traits::destroy(a, this);
            Traits::deallocate(a, this, 1);
        }

        // コピー禁止にする
        ptr_holder(const ptr_holder&) = delete;
        ptr_holder& operator =(const ptr_holder&) = delete;

    private:
        // コンストラクタ
        ptr_holder(T* ptr, Deleter deleter, Alloc alloc)
            :ptr_holder_base(ptr), deleter_(deleter), alloc_(alloc)
        {

        }

    };  // class ptr_holder

    //==========================================================================
    // shared_ptr::make() 用のホルダ
    //==========================================================================
    template<class T, class Alloc>
    class ptr_holder_alloc : public impl::ptr_holder_base {

        // リソースを保持する領域
        typename std::aligned_storage<
            sizeof(T), std::alignment_of<T>::value>::type storage_;
        Alloc alloc_;   // アロケータ

    public:

        void destroy() override { pointer_cast<T*>(&storage_)->~T(); }

        // ホルダ作成
        template<class... Args>
        static ptr_holder_alloc* create_holder(Alloc alloc, Args&&... args)
        {
            // アロケータの再束縛
            struct holder_impl : ptr_holder_alloc<T, Alloc> {
                holder_impl(Alloc a) : ptr_holder_alloc(a) { }
            };
            using Holder = holder_impl;
            using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
            using Traits = std::allocator_traits<Allocator>;
            Allocator a = alloc;

            // ホルダ領域確保
            holder_impl* p = Traits::allocate(a, 1);
            if (p == nullptr) {
                return nullptr;
            }

            // ホルダ構築
            Traits::construct(a, p, alloc);

            // リソース構築
            ::new(&p->storage_) T(std::forward<Args>(args)...);

            return p;
        }

        // ホルダ破棄（自殺するので注意して扱うこと）
        void destroy_holder() override
        {
            // アロケータの再束縛
            using Holder = ptr_holder_alloc<T, Alloc>;
            using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
            using Traits = std::allocator_traits<Allocator>;
            Allocator a = alloc_;

            Traits::destroy(a, this);
            Traits::deallocate(a, this, 1);
        }

        // コピー禁止にする
        ptr_holder_alloc(const ptr_holder_alloc&) = delete;
        ptr_holder_alloc& operator =(const ptr_holder_alloc&) = delete;

    private:
        // コンストラクタ
        ptr_holder_alloc(Alloc alloc)
            :ptr_holder_base(&storage_), alloc_(alloc)
        {

        }

    };  // class ptr_holder_alloc

}   // namespace tork::impl

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
        if (p_holder_) {
            p_holder_->add_ref();
        }
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
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }

    // ポインタ、カスタム削除子、アロケータ設定
    template<class U, class Deleter, class Alloc,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    shared_ptr(U* ptr, Deleter deleter, Alloc alloc)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<U, Deleter, Alloc>;

        p_holder_ = Holder::create_holder(ptr, deleter, alloc);
        if (p_holder_) {
            p_holder_->add_ref();
        }
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
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }

    // ポインタとカスタム削除子設定
    template<class Deleter>
    shared_ptr(T* ptr, Deleter deleter)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<T, Deleter, tork::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, deleter, tork::allocator<void>());
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }

    // ポインタ、カスタム削除子、アロケータ設定
    template<class Deleter, class Alloc>
    shared_ptr(T* ptr, Deleter deleter, Alloc alloc)
        :p_holder_(nullptr)
    {
        using Holder = impl::ptr_holder<T, Deleter, Alloc>;

        p_holder_ = Holder::create_holder(ptr, deleter, alloc);
        if (p_holder_) {
            p_holder_->add_ref();
        }
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
    sptr.p_holder_->add_ref();
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
    sptr.p_holder_->add_ref();
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

