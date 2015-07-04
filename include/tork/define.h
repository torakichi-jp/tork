#ifndef TORK_DEFINE_H_INCLUDED
#define TORK_DEFINE_H_INCLUDED

namespace tork {

// 配列の長さ
template<class T, size_t S>
inline size_t length_of(T (&ary)[S])
{
    return S;
}

}   // namespace tork

#endif  // TORK_DEFINE_H_INCLUDED
