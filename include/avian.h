#ifndef _AVIAN_H_
#define _AVIAN_H_

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

extern int rval;
void alert(char *, ...);
void fatal(int, char *, ...);

char *cleanname(char *);
char *fgetln(FILE *);
char *readcons(char *, char *, int);
int textwidth(void);

uint symmod(uint, char *);

#define stradd(s,...) vstradd(s,__VA_ARGS__,nil)
char *vstradd(char *, ...);

#endif /* _AVIAN_H_ */
