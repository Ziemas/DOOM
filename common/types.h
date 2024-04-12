#ifndef TYPES_H_
#define TYPES_H_

#include <stdbool.h>

#ifdef _EE
typedef long long s64;
typedef unsigned long long u64;
typedef int s32;
typedef unsigned int u32;
typedef short i16;
typedef unsigned short u16;
typedef signed char s8;
typedef unsigned char u8;
#endif

#ifdef _IOP
typedef long long s64;
typedef unsigned long long u64;
typedef long s32;
typedef unsigned long u32;
typedef short i16;
typedef unsigned short u16;
typedef signed char s8;
typedef unsigned char u8;
typedef unsigned int size_t;
#endif

#endif // TYPES_H_
