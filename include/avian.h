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

/* Error handling */
void alert(char *, ...);
void fatal(int, char *, ...);

/* Console I/O */
char *cleanname(char *);
char *fgetln(FILE *);
char *readcons(char *, char *, int);
int textwidth(void);

/* Daemon utilities */
int forkdaemon(void);
void sysalert(char *, ...);
void sysfatal(int, char *, ...);

uint getmode(char *, uint);

char *stradd(char *, ...);

#endif /* _AVIAN_H_ */
