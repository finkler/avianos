#include <u.h>
#include <avian.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

#define RUNPATH "/var/run"

static void __exitdaemon(void);

static char path[PATH_MAX];

int
forkdaemon(void) {
  int fd, pid;

  if(getppid() == 1) {
    errno = EALREADY;
    return -1;
  }
  signal(SIGHUP, SIG_IGN);
  pid = fork();
  if(pid < 0)
    return -1;
  if(pid > 0)
    exit(0);
  umask(0);
  setsid();
  chdir("/");
  freopen("/dev/null", "r", stdin);
  freopen("/dev/null", "w", stdout);
  freopen("/dev/null", "r+", stderr);
  snprintf(path, PATH_MAX, "%s/%s.pid",
    RUNPATH, argv0);
  fd = creat(path, 0644);
  if(fd < 0) {
    if(errno == EEXIST)
      errno = EALREADY;
    return 1;
  }
  dprintf(fd, "%u\n", getpid());
  atexit(__exitdaemon);
  return 0;
}

void
__exitdaemon(void) {
  unlink(path);
}
