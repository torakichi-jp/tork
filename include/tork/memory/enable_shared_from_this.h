//******************************************************************************
//
// enable_shared_from_this
//
//******************************************************************************

#ifndef TORK_MEMORY_ENABLE_SHARED_FROM_THIS_H_INCLUDED
#define TORK_MEMORY_ENABLE_SHARED_FROM_THIS_H_INCLUDED

#include "ptr_holder.h"
#include "shared_ptr.h"
#include "weak_ptr.h"


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
// enable_shared_from_this
//==============================================================================
template<class T>
class enable_shared_from_this {
private:
    weak_ptr<T> weak_this_;
protected:
    enable_shared_from_this() : weak_this_() { }
    enable_shared_from_this(enable_shared_from_this const &) { }
    enable_shared_from_this& operator=(enable_shared_from_this const &) { return *this; }
    ~enable_shared_from_this() { }
public:
    shared_ptr<T> shared_from_this() { return shared_ptr<T>(weak_this_); }
    shared_ptr<T const> shared_from_this() const { return shared_ptr<T const>(weak_this_); }

    template<class T1>
    friend void impl::do_enable_shared(
            enable_shared_from_this<T1>* pEs,
            impl::ptr_holder_base* pHolder);
};

namespace impl {

    template<class T>
    void do_enable_shared(
            enable_shared_from_this<T>* pEs,
            impl::ptr_holder_base* pHolder)
    {
        pEs->weak_this_.p_holder_ = pHolder;
        pEs->weak_this_.p_holder_->add_weak_ref();
    }

    inline void do_enable_shared(const volatile void*, const volatile void*)
    {
        // do nothing
    }

}   // namespace tork::impl

}   // namespace tork

#endif  // TORK_MEMORY_ENABLE_SHARED_FROM_THIS_H_INCLUDED

