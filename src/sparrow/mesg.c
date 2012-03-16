#include <u.h>
#include <avian.h>
#include <sys/stat.h>

void
usage(void) {
  fprint(stderr, "usage: mesg [y|n]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  struct stat sb;
  char *tty;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND 
  if(argc > 1)
    usage();
  
  tty = ttyname(0);
  if(tty == nil)
    fatal(1, "ttyname: %m");
  if(stat(tty, &sb))
    fatal(1, "stat %s: %m", tty);
  if(argc == 1) {
    switch (argv[0][0]) {
    case 'n':
      if(chmod(tty, sb.st_mode&~S_IWGRP))
        fatal(1, "chmod %s: %m", tty);
      exit(1);
    case 'y':
      if(chmod(tty, sb.st_mode|S_IWGRP))
        fatal(1, "chmod %s: %m", tty);
      exit(0);
    }
  } else {
    if(sb.st_mode & S_IWGRP) {
      println("is y");
      exit(0);
    }
    println("is n");
    exit(1);
  }
}
