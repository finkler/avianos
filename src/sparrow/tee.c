#include <u.h>
#include <avian.h>
#include <signal.h>

#define FILE_MAX 100

int
main(int argc, char *argv[]) {
  int aflag, i, n, rval;
  char *buf;
  FILE *f[FILE_MAX];

  aflag = 0;
  ARGBEGIN("ai"){
  case 'a':
    aflag = 1;
    break;
  case 'i':
    signal(SIGINT, SIG_IGN);
    break;
  default:
    fprint(stderr, "usage: tee [-ai] [file...]\n");
    exit(1);
  }ARGEND 
  
  rval = 0;
  n = 0;
  for(i = 0; i < argc && n < FILE_MAX-1; i++) {
    f[n++] = fopen(argv[i], aflag ? "a" : "w");
    if(f[n] == nil) {
      alert("can't open %s: %m", argv[n]);
      rval = 1;
      n--;
    }
  }
  f[n++] = stdout;
  while((buf = fgetln(stdin)))
    for(i = 0; i < n; i++) {
      fprintln(f[i], buf);
      if(ferror(f[i])) {
        alert("error writing %s: %m", argv[i]);
        rval = 1;
      }
    }
  if(ferror(stdin))
    fatal(1, "error reading <stdin>: %m");
  exit(rval);
}
