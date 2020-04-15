#ifndef SIGNALHASH_HPP
#define SIGNALHASH_HPP

#include <cstdint>
#include <string_view>
#include <cstring>
#include <functional>

template<class T, class R, class ... Args>
uint32_t makeHash(T * ptr, R (T::*func)(Args...))
{
  thread_local static char storage[sizeof(func)];
  std::memcpy(storage, &func, sizeof(func));
  const char * data = reinterpret_cast<const char *>(&storage);

  uint32_t hash = std::hash<std::string_view>{}({data, sizeof(func)});
  hash += std::hash<T *>{}(ptr);
  return hash;
}

template<class R, class ... Args>
uint32_t makeHash(R (*func)(Args...))
{
  return std::hash<void *>{}((void *)func);
}

#endif // SIGNALHASH_HPP
