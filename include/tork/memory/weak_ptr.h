//******************************************************************************
//
// ウィークポインタ
//
//******************************************************************************

#ifndef TORK_MEMORY_WEAK_PTR_H_INCLUDED
#define TORK_MEMORY_WEAK_PTR_H_INCLUDED

#include "shared_ptr.h"

namespace tork {

//==============================================================================
// ウィークポインタ
//==============================================================================
template<class T>
class weak_ptr {
    impl::shared_holder_base* p_holder_ = nullptr;

public:

};  // class weak_ptr

}   // namespace tork

#endif  // TORK_MEMORY_WEAK_PTR_H_INCLUDED

