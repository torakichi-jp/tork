#ifndef TORK_MEMORY_ALLOCATOR_H_INCLUDED
#define TORK_MEMORY_ALLOCATOR_H_INCLUDED

#include <new>

namespace tork {

// メモリ確保時に例外を投げないアロケータ
template<class T>
class allocator {
public:
    typedef T value_type;

    allocator() { }
    allocator(const allocator&) { }
    template<class U>
    allocator(const allocator<U>&) { }

    T* allocate(size_t n)
    {
        // 例外を投げない new を呼ぶ
        void* p = ::operator new(sizeof(T) * n, std::nothrow_t());
        return static_cast<T*>(p);
    }

    void deallocate(T* ptr, size_t)
    {
        ::operator delete(ptr);
    }

};

}   // namespace tork

#endif  // TORK_MEMORY_ALLOCATOR_H_INCLUDED
