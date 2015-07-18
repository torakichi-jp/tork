//******************************************************************************
//
// ウィークポインタ
//
//******************************************************************************

#ifndef TORK_MEMORY_WEAK_PTR_H_INCLUDED
#define TORK_MEMORY_WEAK_PTR_H_INCLUDED

#include "shared_ptr.h"

namespace tork {

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
// ウィークポインタ
//==============================================================================
template<class T>
class weak_ptr {
    impl::ptr_holder_base* p_holder_ = nullptr;

    // T じゃない型のにアクセスできるように friend 宣言
    template<class> friend class shared_ptr;
    template<class> friend class weak_ptr;

public:
    typedef T element_type;

    //==========================================================================
    // コンストラクタ

    // デフォルトコンストラクタ
    weak_ptr() : p_holder_(nullptr) { }

    // コピーコンストラクタ
    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    weak_ptr(const weak_ptr<U>& other)
        : p_holder_(other.p_holder_)
    {
        if (p_holder_) {
            p_holder_->add_weak_ref();
        }
    }

    // shared_ptr を受け取るコンストラクタ
    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    weak_ptr(const shared_ptr<U>& other)
        : p_holder_(other.p_holder_)
    {
        if (p_holder_) {
            p_holder_->add_weak_ref();
        }
    }

    // ムーブコンストラクタ
    weak_ptr(weak_ptr&& other)
        : p_holder_(other.p_holder_)
    {
        other.p_holder_ = nullptr;
    }

    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    weak_ptr(weak_ptr<U>&& other)
        : p_holder_(other.p_holder_)
    {
        other.p_holder_ = nullptr;
    }

    // デストラクタ
    ~weak_ptr()
    {
        if (p_holder_) {
            p_holder_->release_weak_ref();
        }
    }

    // コピー代入演算子
    weak_ptr& operator =(const weak_ptr& other)
    {
        if (other.p_holder_ == this->p_holder_) return *this;
        weak_ptr(other).swap(*this);
        return *this;
    }

    // 型変換コピー代入
    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    weak_ptr& operator =(const weak_ptr<U>& other)
    {
        if (other.p_holder_ == this->p_holder_) return *this;
        weak_ptr(other).swap(*this);
        return *this;
    }

    // shared_ptr を受け取る operator =()
    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    weak_ptr& operator =(const shared_ptr<U>& other)
    {
        if (other.p_holder_ == this->p_holder_) return *this;
        weak_ptr(other).swap(*this);
        return *this;
    }

    // ムーブ代入演算子
    weak_ptr& operator =(weak_ptr&& other)
    {
        assert(other.p_holder_ != this->p_holder_);
        weak_ptr(std::move(other)).swap(*this);
        return *this;
    }

    // 型変換ムーブ代入
    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    weak_ptr& operator =(weak_ptr<U>&& other)
    {
        assert(other.p_holder_ != this->p_holder_);
        weak_ptr(std::move(other)).swap(*this);
        return *this;
    }

    // スワップ
    void swap(weak_ptr& other)
    {
        impl::ptr_holder_base* pTmp = other.p_holder_;
        other.p_holder_ = p_holder_;
        p_holder_ = pTmp;
    }

    // リセット
    void reset()
    {
        weak_ptr().swap(*this);
    }

    // 所有者数を返す
    int use_count() const
    {
        return p_holder_ ? p_holder_->get_ref_counter() : 0;
    }

    // 監視している shared_ptr が寿命切れか
    bool expired() const
    {
        return use_count() == 0;
    }

    // shared_ptr の取得
    shared_ptr<T> lock() const
    {
        return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
    }

    template<class T1>
    friend void impl::do_enable_shared(
            enable_shared_from_this<T1>* pEs,
            impl::ptr_holder_base* pHolder);

};  // class weak_ptr

// スワップ非メンバ版
template<class T>
void swap(weak_ptr<T> lhs, weak_ptr<T> rhs)
{
    lhs.swap(rhs);
}

}   // namespace tork

#endif  // TORK_MEMORY_WEAK_PTR_H_INCLUDED

