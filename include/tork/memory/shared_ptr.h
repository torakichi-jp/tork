//******************************************************************************
//
// 参照カウンタ方式資源共有スマートポインタ
//
//******************************************************************************

#ifndef TORK_MEMORY_SHARED_PTR_H_INCLUDED
#define TORK_MEMORY_SHARED_PTR_H_INCLUDED

#include <memory>
#include <type_traits>
#include <cassert>
#include "default_deleter.h"

namespace tork {

// 前方宣言
template<class T>
    class shared_ptr;
template<class T>
    class weak_ptr;

//==============================================================================
// ポインタホルダ基底クラス
//==============================================================================
class shared_holder_base {
    int ref_counter_ = 0;   // 参照カウンタ
    int weak_counter_ = 0;  // ウィークカウンタ

public:
    shared_holder_base() { }
    virtual ~shared_holder_base() { }

    virtual void* get() const = 0;      // ポインタ取得

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
        if (ref_counter_ <= 0) {
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
        if (weak_counter_ <= 0) {
            destroy_holder();
        }
    }

private:
    virtual void destroy() = 0;         // リソース削除
    virtual void destroy_holder() = 0;  // ホルダ自身を削除

};  // class shared_holder_base

//==============================================================================
// ポインタホルダ
//==============================================================================
template<class T, class Deleter, class Alloc>
class shared_holder : public shared_holder_base {
    T* ptr_ = nullptr;  // 保持するポインタ
    Deleter deleter_;   // 削除子
    Alloc alloc_;       // アロケータ
public:

    void* get() const override { return ptr_; }  // ポインタ取得
    void destroy() override { deleter_(ptr_); }  // リソース削除

    // ホルダ作成
    static shared_holder* create_holder(T* ptr, Deleter deleter, Alloc alloc)
    {
        // リソースがなければホルダも作らない
        if (ptr == nullptr) {
            return nullptr;
        }

        // アロケータの再束縛
        using Holder = shared_holder<T, Deleter, Alloc>;
        using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
        using Traits = std::allocator_traits<Allocator>;
        Allocator a = alloc;

        shared_holder* p = Traits::allocate(a, 1);
        if (p == nullptr) {
            // ホルダの領域を確保できなかったら、リークを防ぐため
            // リソースを解放しておく
            deleter(ptr);
            return nullptr;
        }
        // Traits::construct() では private コンストラクタにアクセスできないので
        // 直接 placement new を呼ぶ
        //Traits::construct(a, p, ptr, deleter, alloc);
        p = ::new(p) shared_holder(ptr, deleter, alloc);
        return p;
    }

    // ホルダ破棄（自殺するので注意して扱うこと）
    void destroy_holder() override
    {
        // アロケータの再束縛
        using Holder = shared_holder<T, Deleter, Alloc>;
        using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
        using Traits = std::allocator_traits<Allocator>;
        Allocator a = alloc_;

        Traits::destroy(a, this);
        Traits::deallocate(a, this, 1);
    }

    // コピー禁止にする
    shared_holder(const shared_holder&) = delete;
    shared_holder& operator =(const shared_holder&) = delete;

private:
    // コンストラクタ
    shared_holder(T* ptr, Deleter deleter, Alloc alloc)
        :ptr_(ptr), deleter_(deleter), alloc_(alloc)
    {

    }

};  // class shared_holder

//==============================================================================
// 参照カウンタ式スマートポインタ
//==============================================================================
template <class T>
class shared_ptr {
    shared_holder_base* p_holder_ = nullptr;

    // T じゃない型のにアクセスできるように friend 宣言
    template<class U> friend class shared_ptr;
    template<class U> friend class weak_ptr;

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
        using Holder = shared_holder<U, default_deleter<U>, std::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, default_deleter<U>(), std::allocator<void>());
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
        using Holder = shared_holder<U, Deleter, std::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, deleter, std::allocator<void>());
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
        using Holder = shared_holder<U, Deleter, Alloc>;

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
    shared_ptr(const weak_ptr<U>& other)
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
        shared_holder_base* pTmp = this->p_holder_;
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

};  // class shared_ptr

//==============================================================================
// shared_ptr の配列形式の特殊化
//==============================================================================
template <class T>
class shared_ptr<T[]> {
    shared_holder_base* p_holder_ = nullptr;

    // T じゃない型のにアクセスできるように friend 宣言
    template<class U> friend class shared_ptr;
    template<class U> friend class weak_ptr;

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
        using Holder = shared_holder<U, default_deleter<U[]>, std::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, default_deleter<U>(), std::allocator<void>());
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
        using Holder = shared_holder<U, Deleter, std::allocator<void>>;

        p_holder_ = Holder::create_holder(
                ptr, deleter, std::allocator<void>());
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
        using Holder = shared_holder<U, Deleter, Alloc>;

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
    shared_ptr(const weak_ptr<U>& other)
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
        shared_holder_base* pTmp = this->p_holder_;
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

};  // class shared_ptr



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


}   // namespace tork

#endif  // TORK_MEMORY_SHARED_PTR_H_INCLUDED

