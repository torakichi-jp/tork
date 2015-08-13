
#ifndef TORK_ALGORITHM_H_INCLUDED
#define TORK_ALGORITHM_H_INCLUDED

namespace tork {

// for_each() の添え字付き版
template<class InputIter, class Function>
Function each_with_index(InputIter first, InputIter last, Function f)
{
    size_t i = 0;
    for (InputIter it = first; it != last; ++it) {
        f(*it, i);
        ++i;
    }
    return std::move(f);
}

}   // namespace tork

#endif  // TORK_ALGORITHM_H_INCLUDED

