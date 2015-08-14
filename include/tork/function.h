
#ifndef TORK_FUNCTION_H_INCLUDED
#define TORK_FUNCTION_H_INCLUDED

#include <functional>

namespace tork {

template<class T>
class enumerate : public std::unary_function<T, void> {
    typedef unsigned long ul;

    ul index_;
    std::function<void(T, ul)> func_;
public:
    enumerate(std::function<void(T, ul)> f)
        :index_(0), func_(f)
    {

    }
    enumerate(ul i, std::function<void(T, ul)> f)
        :index_(i), func_(f)
    {

    }

    void operator()(T t)
    {
        func_(t, index_);
        ++index_;
    }

};  // class enumerate

}   // namespace tork

#endif  // TORK_FUNCTION_H_INCLUDED
