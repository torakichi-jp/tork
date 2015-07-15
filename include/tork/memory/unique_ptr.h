//******************************************************************************
//
// 所有権移動スマートポインタ
//
//******************************************************************************
#ifndef TORK_MEMORY_UNIQUE_PTR_INCLUDED
#define TORK_MEMORY_UNIQUE_PTR_INCLUDED

#include "ptr_holder.h"

namespace tork {

template<class T, class D = default_deleter<T>>
class unique_ptr {
public:
    typedef T element_type;
    typedef D deleter_type;
    typedef T* pointer;

private:
    pointer ptr_ = nullptr;
    deleter_type deleter_;

public:

    // デフォルトコンストラクタ
    unique_ptr() : ptr_(nullptr) { }

    // ポインタを受け取るコンストラクタ
    explicit unique_ptr(pointer ptr)
        :ptr_(ptr), deleter_(default_deleter<T>()) { }

    // ポインタと削除子
    unique_ptr(pointer ptr, const D& deleter)
        :ptr_(ptr), deleter_(deleter) { }
    unique_ptr(pointer ptr, D&& deleter)
        :ptr_(ptr), deleter_(std::move(deleter)) { }

    // nullptr
    explicit unique_ptr(nullptr_t) :p_holder_(nullptr) { }

    // ムーブコンストラクタ
    unique_ptr(unique_ptr&& other)
        :ptr_(other.ptr_), deleter_(std::move(other.deleter_))
    {
        other.ptr_ = nullptr;
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

};  // class unique_ptr

}   // namespace tork

#endif  // TORK_MEMORY_UNIQUE_PTR_INCLUDED

