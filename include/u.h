#ifndef _U_H_
#define _U_H_

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define max(a,b)      ((a)>(b)?(a):(b))
#define min(a,b)      ((a)<(b)?(a):(b))
#define nelem(x)      (sizeof(x)/sizeof((x)[0]))
#define nil           ((void *)0)

#define print(s)      fputs((s),stdout)
#define println       puts
#define fprint(f,s)   fputs((s),(f))

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long uvlong;
typedef long long vlong;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#endif /* _U_H_ */
