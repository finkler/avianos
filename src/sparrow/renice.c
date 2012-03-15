#include <avian.h>
#include <errno.h>
#include <pwd.h>
#include <sys/resource.h>

void
usage(void) {
  fprint("usage: renice [-g|-p|-u] -n increment ID...\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int gflag, pflag, uflag;
  int i, n, rval, which;
  struct passwd *pwd;
  id_t who;

  gflag = pflag = uflag = 0;
  n = which = 0;
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
  if(gflag+pflag+uflag == 0)
    which = PRIO_PROCESS;
  for(i = 0; i < argc; i++) {
    if(uflag && !isdigit(argv[i][0])) {
      if(!(pwd = getpwnam(argv[i]))) {
        alert("unknown user %s", argv[i]);
        rval = 1;
        continue;
      }
      who = pwd->pw_uid;
    } else who = atoi(argv[i]);
    errno = 0;
    n += getpriority(which, who);
    if(errno)
      fatal(1, "getpriority: %m");
    if(setpriority(which, who, n))
      fatal(1, "setpriority: %m");
  }
  exit(rval);
}
