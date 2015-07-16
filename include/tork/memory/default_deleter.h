//******************************************************************************
//
// スマートポインタのデフォルト削除子
//
//******************************************************************************
#ifndef TORK_MEMORY_DEFAULT_DELETER_H_INCLUDED
#define TORK_MEMORY_DEFAULT_DELETER_H_INCLUDED

#include <type_traits>

namespace tork {

// 通常のdelete用
template<class T>
struct default_deleter {

    typedef T* pointer;

    default_deleter() { }
    ~default_deleter() { }

    template<class U,
        class = typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type>
    default_deleter(const default_deleter<U>&)
    {

    }

    void operator ()(pointer ptr) const
    {
        static_assert(sizeof(T) > 0,
                "can't delete an incomplete type");
        delete ptr;
    }
};

// 配列形式の特殊化
template<class T>
struct default_deleter<T[]> {

    typedef T* pointer;

    default_deleter() { }
    ~default_deleter() { }

    void operator ()(pointer ptr) const
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

