#include <avian.h>
#include <signal.h>

#define FILE_MAX 100

int
main(int argc, char *argv[]) {
  int aflag, i, n, rval;
  char buf[LINE_MAX];
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
    fprint("usage: tee [-ai] [file...]\n", stderr);
    exit(1);
  }ARGEND 
  rval = 0;
  for(n = 0; n < argc && n < FILE_MAX-1; n++)
    if(!(f[n] = fopen(argv[n], aflag ? "a" : "w"))) {
      alert("can't open %s: %m", argv[n]);
      rval = 1;
    }
  f[n++] = stdout;
  while(fgets(buf, LINE_MAX, stdin))
    for(i = 0; i < n; i++) {
      fprint(buf, f[i]);
      if(ferror(f[i])) {
        alert("error writing %s: %m", argv[i]);
        rval = 1;
      }
    }
  if(ferror(stdin))
    fatal(1, "error reading <stdin>: %m");
  exit(rval);
}
