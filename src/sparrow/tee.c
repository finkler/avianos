#include <u.h>
#include <avian.h>
#include <signal.h>

#define FILE_MAX 100

int
main(int argc, char *argv[]) {
  int aflag, i, n;
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

  for(i = n = 0; i < argc && n < FILE_MAX-1; i++, n++) {
    f[n] = fopen(argv[i], aflag?"a":"w");
    if(f[n] == nil) {
      alert("open %s: %m", argv[i]);
      n--;
    }
  }
  f[n++] = stdout;
  while((buf = fgetln(stdin)))
    for(i = 0; i < n; i++) {
      fprintln(f[i], buf);
      if(ferror(f[i]))
        alert("write %s: %m", argv[i]);
    }
  if(ferror(stdin))
    fatal(1, "read <stdin>: %m");
  exit(rval);
}
