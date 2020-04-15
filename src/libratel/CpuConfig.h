#ifndef CPUCONFIG_H
#define CPUCONFIG_H

#define RATEL_BIG_ENDIAN    1
#define RATEL_LITTLE_ENDIAN 2
#if defined(__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define RATEL_BYTE_ORDER RATEL_BIG_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define RATEL_BYTE_ORDER RATEL_LITTLE_ENDIAN
#endif /* __BYTE_ORDER__ */
#elif defined(__BYTE_ORDER)
#if __BYTE_ORDER == __BIG_ENDIAN
#define RATEL_BYTE_ORDER RATEL_BIG_ENDIAN
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define RATEL_BYTE_ORDER RATEL_LITTLE_ENDIAN
#endif /* __BYTE_ORDER */
#elif defined(__BIG_ENDIAN__)
#define RATEL_BYTE_ORDER RATEL_BIG_ENDIAN
#elif defined(__LITTLE_ENDIAN__)
#define RATEL_BYTE_ORDER RATEL_LITTLE_ENDIAN
#endif
#if !defined(RATEL_BYTE_ORDER)
#error Unknown endianness.
#endif

#endif // CPUCONFIG_H
