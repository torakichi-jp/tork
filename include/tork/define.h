#ifndef TORK_DEFINE_H_INCLUDED
#define TORK_DEFINE_H_INCLUDED

namespace tork {

// 例外を投げない指定
#define NO_EXCEPT   throw()

    // 配列の長さ
    template<class T, size_t S>
    inline size_t length_of(T (&ary)[S])
    {
        return S;
    }

    // ポインタキャスト
    template<class T>
    inline T pointer_cast(void* ptr) {
        return static_cast<T>(ptr);
    }
    template<class T>
    inline T pointer_cast(const void* ptr) {
        return static_cast<T>(ptr);
    }

    // & 演算子が再定義されていてもアドレス取得
    template<class T>
    inline T* address_of(T& obj) {
        return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(obj)));
    }

}   // namespace tork

#endif  // TORK_DEFINE_H_INCLUDED
