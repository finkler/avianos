#include <u.h>
#include <avian.h>
#include <time.h>

void
usage(void) {
  fprint(stderr, "usage: date [-u] [+format]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  char buf[512], *fmt;
  long now;
  int uflag;

  uflag = 0;
  ARGBEGIN("u"){
  case 'u':
    uflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc > 1)
    usage();
  fmt = "%a %b %e %H:%M:%S %Z %Y";
  if(argc == 1) {
    if(argv[0][0] != '+')
      usage();
    fmt = &argv[0][1];
  }
  now = time(nil);
  strftime(buf, sizeof buf, fmt, uflag?gmtime(&now):localtime(&now));
  println(buf);
}
