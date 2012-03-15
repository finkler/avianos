#include <avian.h>

void
usage(void) {
  fprint("usage: tty\n", stderr);
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
  if(!(p = ttyname(0))) {
    println("not a tty");
    exit(1);
  }
  println(p);
}
