//******************************************************************************
//
// 所有権移動スマートポインタ
//
//******************************************************************************
#ifndef TORK_MEMORY_UNIQUE_PTR_INCLUDED
#define TORK_MEMORY_UNIQUE_PTR_INCLUDED

#include "default_deleter.h"

namespace tork {

    template<class T, class D = tork::default_deleter<T>>
    class unique_ptr {
    public:
        typedef T* pointer;
        typedef T element_type;
        typedef D deleter_type;

    private:
        pointer ptr_;           // 保持するポインタ
        deleter_type deleter_;  // 削除子

    };  // class unique_ptr

}   // namespace tork

#endif  // TORK_MEMORY_UNIQUE_PTR_INCLUDED

