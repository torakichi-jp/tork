//******************************************************************************
//
// 配列
//
//******************************************************************************

#ifndef TORK_ARRAY_H_INCLUDED
#define TORK_ARRAY_H_INCLUDED

#include <memory>

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


    ArrayBase(const allocator_type& a, size_type n)
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
template<class T, class Allocator = std::allocator<T>>
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
        if (p_base_) {
            for (size_type i = 0; i < p_base_->size_; ++i) {
                AllocTraits::destroy(p_base_->alloc_, &p_base_->data_[i]);
            }

            using traits = AllocTraits::rebind_traits<Base>;
            AllocTraits::rebind_alloc<Base> a = p_base_->alloc_;

            traits::destroy(a, p_base_);
            traits::deallocate(a, p_base_, sizeof(Base));
        }
    }

    void reserve(size_type s)
    {
        if (p_base_ && s <= p_base_->capacity_) return;

        using traits = AllocTraits::rebind_traits<Base>;
        AllocTraits::rebind_alloc<Base> a = p_base_->alloc_;

        Base* p = traits::allocate(a, sizeof(Base));
        if (p == nullptr) return;

        try {
            traits::construct(a, p, p_base_->alloc_, s);
            if (p->data_ == nullptr) return;

            if (p_base_) {
                T* pData = p_base_->data_;
                size_type sz = p_base_->size_;

                std::uninitialized_copy(pData, &pData[sz], p->data_);

                for (size_type i = 0; i < sz; ++i) {
                    AllocTraits::destroy(p_base_->alloc_, &pData[i]);
                }
                p->size_ = sz;
            }
            p_base_ = p;
        }
        catch (...) {
            traits::deallocate(a, p, sizeof(Base));
            throw;
        }
    }

};  // class Array

}   // namespace tork

#endif  // TORK_ARRAY_H_INCLUDED

