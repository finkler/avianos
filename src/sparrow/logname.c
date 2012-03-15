#include <avian.h>

void
usage(void) {
  fprint("usage: logname\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *p;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND 
  if(argc != 0)
    usage();
  if(!(p = getlogin()))
    fatal(1, "getlogin: %m");
  println(p);
}
