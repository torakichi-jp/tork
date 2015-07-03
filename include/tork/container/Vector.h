//******************************************************************************
// operator[] で例外を投げる単純なvector
//
// by ストラウストラップのプログラミング入門 19.4.2
//******************************************************************************
#ifndef TORK_CONTAINER_VECTOR_H_INCLUDED
#define TORK_CONTAINER_VECTOR_H_INCLUDED

#include <stdexcept>
#include <vector>

namespace tork {

    // Vector用拡張範囲エラー報告
    struct RangeError : std::out_of_range {
        int index;
        RangeError(int i): std::out_of_range("Range Error"), index(i) { }
    };

    template<class T>
    class Vector : public std::vector<T> {
    public:
        typedef typename std::vector<T>::size_type size_type;

        Vector() { }
        explicit Vector(size_type n) : std::vector<T>(n) { }
        Vector(size_type n, const T& v) : std::vector<T>(n, v) { }

        T& operator[] (size_type i)
        {
            if (i < 0 || this->size() <= i) {
                throw RangeError(i);
            }
            return std::vector<T>::operator[](i);
        }

        const T& operator[] (size_type i) const
        {
            if (i < 0 || this->size() <= i) {
                throw RangeError(i);
            }
            return std::vector<T>::operator[](i);
        }
    };

}   // namespace tork

#endif  // TORK_CONTAINER_VECTOR_H_INCLUDED

