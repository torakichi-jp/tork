//******************************************************************************
// operator[] で例外を投げる単純なvector
//
// by ストラウストラップのプログラミング入門 19.4.2
//******************************************************************************
#ifndef TORK_CONTAINER_VECTOR_H_INCLUDED
#define TORK_CONTAINER_VECTOR_H_INCLUDED

#include <stdexcept>
#include <vector>
#include "../memory/allocator.h"

namespace tork {

    // Vector用拡張範囲エラー報告
    struct OutOfRangeError : std::out_of_range {
        int index;
        OutOfRangeError(int i): std::out_of_range("Range Error"), index(i) { }
    };

    template<class T, class Allocator = std::allocator<T>>
    class Vector : public std::vector<T, Allocator> {
    public:
        typedef std::vector<T, Allocator> base_type;
        typedef typename base_type::size_type size_type;

        Vector() { }
        explicit Vector(size_type n) : base_type(n) { }
        Vector(size_type n, const T& v) : base_type(n, v) { }

        T& operator[] (size_type i)
        {
            if (i < 0 || this->size() <= i) {
                throw OutOfRangeError(i);
            }
            return base_type::operator[](i);
        }

        const T& operator[] (size_type i) const
        {
            if (i < 0 || this->size() <= i) {
                throw OutOfRangeError(i);
            }
            return base_type::operator[](i);
        }
    };

}   // namespace tork

#endif  // TORK_CONTAINER_VECTOR_H_INCLUDED

