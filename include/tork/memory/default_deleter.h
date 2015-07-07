//******************************************************************************
//
// スマートポインタのデフォルト削除子
//
//******************************************************************************
#ifndef TORK_MEMORY_DEFAULT_DELETER_H_INCLUDED
#define TORK_MEMORY_DEFAULT_DELETER_H_INCLUDED

#include "../define.h"
#include <type_traits>

namespace tork {

// 通常のdelete用
template<class T>
struct default_deleter {
    default_deleter() NO_EXCEPT { }
    ~default_deleter() { }

    template<class U>
    default_deleter(const default_deleter<U>&) NO_EXCEPT
    {
        static_assert(std::is_convertible<U*, T*>::value,
                "specified an inconvertible type");
    }

    void operator ()(T* ptr) const
    {
        static_assert(sizeof(T) > 0,
                "can't delete an incomplete type");
        delete ptr;
    }
};

// 配列形式の特殊化
template<class T>
struct default_deleter<T[]> {
    default_deleter() NO_EXCEPT { }
    ~default_deleter() { }

    void operator ()(T* ptr) const
    {
        static_assert(sizeof(T) > 0,
                "can't delete an incomplete type");
        delete[] ptr;
    }

    template<class U>
    void operator()(U* ptr) const = delete;
};

}   // namespace tork

#endif  // TORK_MEMORY_DEFAULT_DELETER_H_INCLUDED

