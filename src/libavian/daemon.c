#include <avian.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define RUNPATH "/var/run"

static void __exitdaemon(void);

static char *path[PATH_MAX];

int
forkdaemon(void) {
  int fd;
  struct flock fl;
  pid_t pid;

  if(getppid() == 1) {
    errno = EALREADY;
    return -1;
  }
  pid = fork();
  if(pid < 0)
    return -1;
  if(pid > 0)
    exit(0);
  umask(0);
  if(setsid() == -1 || chdir("/")) ||
    !freopen("/dev/null", "r", stdin) ||
    !freopen("/dev/null", "w", stdout) ||
    !freopen("/dev/null", "r+", stderr))
    return 1;
  snprintf(path, PATH_MAX, "%s/%s.pid", _RUNPATH, argv0);
  fd = creat(path, 0644);
  if(fd < 0)
    return 1;
  fl.l_start = 0;
  fl.l_len = 0;
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  if(fcntl(fd, F_SETLK, &fl) < 0) {
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
