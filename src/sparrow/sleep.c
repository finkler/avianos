#include <avian.h>
#include <signal.h>

void
usage(void) {
  fprint("usage: sleep time\n", stderr);
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
  signal(SIGALRM, SIG_IGN);
  if(sleep(atoi(*argv)))
    exit(1);
}
