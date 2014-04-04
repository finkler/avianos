#include <u.h>
#include <avian.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

#define RUNPATH "/var/run"

static void __exitdaemon(void);

char *path;

int
bg(void)
{
  int fd, pid;

  if(getppid() == 1){
    errno = EALREADY;
    return -1;
  }
  signal(SIGHUP, SIG_IGN);
  pid = fork();
  if(pid < 0)
    return -1;
  if(pid > 0)
    exit(0);
  path = stradd(RUNPATH"/", argv0, ".pid");
  fd = open(path, O_WRONLY|O_CREAT|O_EXCL, 0644);
  if(fd < 0){
    if(errno == EEXIST)
      errno = EALREADY;
    return 1;
  }
  dprintf(fd, "%u\n", getpid());
  atexit(__exitdaemon);
  umask(0);
  setsid();
  chdir("/");
  freopen("/dev/null", "r", stdin);
  freopen("/dev/null", "w", stdout);
  freopen("/dev/null", "r+", stderr);
  return 0;
}

void
__exitdaemon(void)
{
  unlink(path);
}
