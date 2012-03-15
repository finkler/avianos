#include <avian.h>
#include <libgen.h>

void
usage(void) {
  fprint(stderr, "usage: dirname string\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  ARGBEGIN(""){
  default:
    usage();
  }ARGEND 
  
  if(argc != 1)
    usage();
  println(dirname(argv[0]));
}
