//******************************************************************************
//
// 所有権移動スマートポインタ
//
//******************************************************************************
#ifndef TORK_MEMORY_UNIQUE_PTR_INCLUDED
#define TORK_MEMORY_UNIQUE_PTR_INCLUDED

#include "ptr_holder.h"

namespace tork {

    template<class T>
    class unique_ptr {
        impl::ptr_holder_base* p_holder_;

    public:
        typedef T* pointer;
        typedef T element_type;

    };  // class unique_ptr

}   // namespace tork

#endif  // TORK_MEMORY_UNIQUE_PTR_INCLUDED

