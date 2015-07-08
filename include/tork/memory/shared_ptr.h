//******************************************************************************
//
// 参照カウンタ方式資源共有スマートポインタ
//
//******************************************************************************

#ifndef TORK_MEMORY_SHARED_PTR_H_INCLUDED
#define TORK_MEMORY_SHARED_PTR_H_INCLUDED

#include <memory>
#include <type_traits>
#include "default_deleter.h"

namespace tork {

//==============================================================================
// ポインタホルダ基底クラス
//==============================================================================
class shared_holder_base {
    int ref_counter_ = 0;   // 参照カウンタ
    int weak_counter_ = 0;  // ウィークカウンタ
public:
    shared_holder_base() { }
    virtual ~shared_holder_base() { }

    // メンバにアクセスできるのは shared_ptr と weak_ptr だけにする
    template<class> friend class shared_ptr;
    template<class> friend class weak_ptr;

private:
    virtual void* get() const = 0;      // ポインタ取得
    virtual void destroy() = 0;         // リソース削除
    virtual void destroy_holder() = 0;  // ホルダ自身を削除

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

};

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
        // アロケータの再束縛
        using Holder = shared_holder<T, Deleter, Alloc>;
        using Allocator = std::allocator_traits<Alloc>::rebind_alloc<Holder>;
        using Traits = std::allocator_traits<Allocator>;
        Allocator a = alloc;

        shared_holder* p = Traits::allocate(a, 1);
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

    // コピー禁止する
    shared_holder(const shared_holder&) = delete;
    shared_holder& operator =(const shared_holder&) = delete;

private:
    // コンストラクタ
    shared_holder(T* ptr, Deleter deleter, Alloc alloc)
        :ptr_(ptr), deleter_(deleter), alloc_(alloc)
    {

    }

};

//==============================================================================
// 参照カウンタ式スマートポインタ
//==============================================================================
template <class T>
class shared_ptr {
    shared_holder_base* p_holder_ = nullptr;

public:
    typedef T element_type; // 要素型

    //--------------------------------------------------------------------------
    // コンストラクタ

    // デフォルト
    shared_ptr(): p_holder_(nullptr) { }

    // ポインタ設定
    template<class U>
    shared_ptr(U* ptr)
        :p_holder_(
                shared_holder<U, default_deleter<U>, std::allocator<void>>::create_holder(
                    ptr, default_deleter<U>(), std::allocator<void>()
                ))
    {
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }

    // ポインタとカスタム削除子設定
    template<class U, class Deleter>
    shared_ptr(U* ptr, Deleter deleter)
        :p_holder_(
                shared_holder<U, Deleter, std::allocator<void>>::create_holder(
                    ptr, Deleter(), std::allocator<void>()
                ))
    {
        if (p_holder_) {
            p_holder_->add_ref();
        }
    }

    // ポインタ、カスタム削除子、アロケータ設定
    template<class U, class Deleter, class Alloc>
    shared_ptr(U* ptr, Deleter deleter, Alloc alloc)
        :p_holder_(
                shared_holder<U, Deleter, Alloc>::create_holder(
                    ptr, deleter, alloc
                ))
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

};  // class shared_ptr


}   // namespace tork

#endif  // TORK_MEMORY_SHARED_PTR_H_INCLUDED

