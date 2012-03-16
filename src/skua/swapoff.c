#include <u.h>
#include <avian.h>
#include <sys/swap.h>

void
usage(void) {
  fprint(stderr, "usage: swapoff file...\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int i, rval;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND 
  
  if(argc < 1)
    usage();
  rval = 0;
  for(i = 0; i < argc; i++)
    if(swapoff(argv[i])) {
      alert("%m");
      rval = 1;
    }
  exit(rval);
}
