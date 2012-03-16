#include <u.h>
#include <avian.h>
#include <errno.h>
#include <pwd.h>
#include <sys/resource.h>

void
usage(void) {
  fprint(stderr, "usage: renice [-g|-p|-u] -n increment ID...\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int gflag, pflag, uflag;
  int i, n, rval, which, who;
  struct passwd *pwd;

  gflag = pflag = uflag = 0;
  n = 0;
  which = PRIO_PROCESS;
  ARGBEGIN("gn:pu"){
  case 'g':
    which = PRIO_PGRP;
    gflag = 1;
    break;
  case 'p':
    which = PRIO_PROCESS;
    pflag = 1;
    break;
  case 'n':
    n = atoi(optarg);
    break;
  case 'u':
    which = PRIO_USER;
    uflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  
  if(argc < 1 || gflag+pflag+uflag > 1 || n == 0)
    usage();
  rval = 0;
  for(i = 0; i < argc; i++) {
    if(uflag && !isdigit(argv[i][0])) {
      pwd = getpwnam(argv[i]);
      if(pwd == nil) {
        alert("unknown user %s", argv[i]);
        rval = 1;
        continue;
      }
      who = pwd->pw_uid;
    } else {
      who = atoi(argv[i]);
    }
    errno = 0;
    n += getpriority(which, who);
    if(errno || setpriority(which, who, n)) {
      alert("%m");
      rval = 1;
    }
  }
  exit(rval);
}
