#ifndef _AVIAN_H_
#define _AVIAN_H_

extern char *argv0;
void __fixargv0(char *);
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

uint longlen(vlong);

uint symmod(uint, char *);

#define stradd(s,...) vstradd(s,__VA_ARGS__,nil)
char *vstradd(char *, ...);
char *strsub(char *, char *, char *);
#define trim(s) ltrim(rtrim(s))
char *ltrim(char *);
char *rtrim(char *);

#endif /* _AVIAN_H_ */
