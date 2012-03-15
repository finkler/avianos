#include <avian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#define FILENAME  "nohup.out"

void
usage(void) {
  fprint("usage: nohup utility [argument...]\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int fd;
  char path[PATH_MAX];

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND 
  if(argc < 1)
    usage();
  if(isatty(1)) {
    strcpy(path, FILENAME);
    if((fd = open(path, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR)) < 0) {
      snprintf(path, PATH_MAX, "%s/%s", getenv("HOME"), FILENAME);
      if((fd = open(path, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR)) < 0)
        fatal(1, "can't open %s: %m", path);
    }
    if(dup2(fd, 1) < 0)
      fatal(1, "%m");
    alert("sending output to %s", path);
  }
  if(isatty(2) && dup2(1, 2) < 0)
    fatal(1, "%m");
  signal(SIGHUP, SIG_IGN);
  execvp(*argv, argv);
  fatal(errno == ENOENT ? 127 : 126, "can't exec %s: %m", *argv);
}
