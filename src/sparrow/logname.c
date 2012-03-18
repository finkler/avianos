#include <u.h>
#include <avian.h>

void
usage(void) {
  fprint(stderr, "usage: logname\n");
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
  p = getlogin();
  if(p == nil)
    fatal(1, "%m");
  println(p);
}
