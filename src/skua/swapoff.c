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
  int i;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND

  if(argc < 1)
    usage();
  for(i = 0; i < argc; i++)
    if(swapoff(argv[i]))
      alert("swapoff: %m");
  exit(rval);
}
