#include <avian.h>
#include <errno.h>
#include <sys/resource.h>

void
usage(void) {
  fprint("usage: nice [-n increment] utility [argument...]\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int n;

  n = 10;
  ARGBEGIN("n:"){
  case 'n':
    n = atoi(optarg);
    break;
  default:
    usage();
  }ARGEND 
  if(argc == 0)
    usage();
  errno = 0;
  n += getpriority(PRIO_PROCESS, 0);
  if(errno)
    fatal(1, "getpriority: %m");
  if(setpriority(PRIO_PROCESS, 0, n))
    fatal(1, "setpriority: %m");
  execvp(*argv, argv);
  fatal(errno == ENOENT ? 127 : 126, "can't exec %s: %m", argv[0]);
}
