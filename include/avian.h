#ifndef _AVIAN_H_
#define _AVIAN_H_

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

extern void __fixargv0(char *);
extern char *argv0;
#define ARGBEGIN(s)\
  setlocale(LC_ALL, "");\
  __fixargv0(argv[0]);\
  opterr = 0;\
  {\
  int c;\
  while((c = getopt(argc, argv, (s))) != -1)\
    switch(c)
#define ARGEND\
  }\
  argc -= optind;\
  argv += optind;

#define max(a,b)      ((a)>(b)?(a):(b))
#define min(a,b)      ((a)<(b)?(a):(b))
#define nelem(x)      (sizeof(x)/sizeof((x)[0]))
#define nil           ((void *)0)
#define println       puts
#define fprintln(f,s) fputs((s),(f));fputc('\n',(f))
#define print(s)      fputs((s),stdout)
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

extern char *fgetln(FILE *);

extern void alert(char *, ...);
extern void fatal(int, char *, ...);

extern void sysalert(char *, ...);
extern void sysfatal(int, char *, ...);

extern char *cleanname(char *);
extern char *readcons(char *, char *, int);
extern int textwidth(void);

extern int forkdaemon(void);

#endif /* _AVIAN_H_ */