//******************************************************************************
//
// 配列
//
//******************************************************************************

#ifndef TORK_ARRAY_H_INCLUDED
#define TORK_ARRAY_H_INCLUDED

#include <memory>
#include "../memory/allocator.h"

namespace tork {

    namespace impl {

template<class T, class A>
struct ArrayBase {
    typedef size_t size_type;
    typedef A allocator_type;

    typedef std::allocator_traits<allocator_type> alloc_traits;

    T* data_ = nullptr;
    size_type capacity_ = 0;
    size_type size_ = 0;
    allocator_type alloc_;


    ArrayBase(allocator_type& a, size_type n)
        :alloc_(a), data_(alloc_traits::allocate(a, n)),
        size_(0), capacity_(n)
    {

    }

    ~ArrayBase()
    {
        alloc_traits::deallocate(alloc_, data_, capacity_);
    }

};  // class ArrayBase


    }   // namespace tork::impl

//==============================================================================
// 配列クラス
//==============================================================================
template<class T, class Allocator = tork::allocator<T>>
class Array {
public:
    typedef impl::ArrayBase<T, Allocator> Base;

    typedef typename Base::size_type size_type;
    typedef typename Base::allocator_type allocator_type;

    typedef std::allocator_traits<allocator_type> AllocTraits;

private:
    Base* p_base_ = nullptr;

public:

    Array() { }

    ~Array()
    {
        destroy_base(p_base_);
    }

    // 容量の予約
    void reserve(size_type s)
    {
        if (p_base_ == nullptr) {
            // 空だったらベースを作る
            p_base_ = create_base(s, allocator_type());
            return;
        }
        else if (s <= capacity()) {
            // 指定された容量が現在の容量よりも小さければ
            // 何もしない
            return;
        }

        Base* p = create_base(s, p_base_->alloc_);
        if (p == nullptr || p->data_ == nullptr) return;
        try {
            // 要素のムーブ
            for (size_type i = 0; i < size(); ++i) {
                AllocTraits::construct(
                        p->alloc_, &p->data_[i], std::move(p_base_->data_[i]));
            }
            p->size_ = size();

            // 古い要素の削除
            destroy_base(p_base_);

            p_base_ = p;
        }
        catch (...) {
            destroy_base(p);
            throw;
        }
    }

    // 容量
    size_type capacity() const
    {
        return p_base_ ? p_base_->capacity_ : 0;
    }

    // 要素数
    size_type size() const
    {
        return p_base_ ? p_base_->size_ : 0;
    }

private:

    // 配列ベース作成
    Base* create_base(size_type s, allocator_type& alloc)
    {
        using traits = AllocTraits::rebind_traits<Base>;
        AllocTraits::rebind_alloc<Base> a = alloc;
        Base* p = traits::allocate(a, sizeof(Base));
        try {
            traits::construct(a, p, alloc, s);
        }
        catch (...) {
            traits::deallocate(a, p, sizeof(Base));
            throw;
        }
        return p;
    }

    // 配列ベース破棄
    void destroy_base(Base* p)
    {
        if (p) {
            for (size_type i = 0; i < p->size_; ++i) {
                AllocTraits::destroy(p->alloc_, &p->data_[i]);
            }

            using traits = AllocTraits::rebind_traits<Base>;
            AllocTraits::rebind_alloc<Base> a = p->alloc_;

            traits::destroy(a, p);
            traits::deallocate(a, p, sizeof(Base));
        }
    }

};  // class Array

}   // namespace tork

#endif  // TORK_ARRAY_H_INCLUDED

