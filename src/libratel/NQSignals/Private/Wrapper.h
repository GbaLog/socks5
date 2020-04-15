#ifndef WRAPPER_H
#define WRAPPER_H

#include <functional>

template<class T, class R, class ... Args>
auto makeWrapper(T * ptr, R (T::*func)(Args...))
{
  return [ptr, func] (Args &&... args) -> R
  { return (ptr->*func)(std::forward<Args>(args)...); };
}

#endif //WRAPPER_H
