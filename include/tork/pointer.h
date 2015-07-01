//******************************************************************************
// ポインタ処理
//******************************************************************************

#ifndef TORK_POINTER_H_INCLUDED
#define TORK_POINTER_H_INCLUDED

namespace tork {

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

#endif  // TORK_POINTER_H_INCLUDED

