#include <u.h>
#include <avian.h>

void
usage(void) {
  fprint(stderr, "usage: tty\n");
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
  p = ttyname(0);
  if(p == nil) {
    println("not a tty");
    exit(1);
  }
  println(p);
}
