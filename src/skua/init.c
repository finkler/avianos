#include <u.h>
#include <avian.h>
#include <signal.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define LOGIN    "/bin/login"
#define PATH     "/bin"
#define RC_START "/etc/rc.start"
#define RC_STOP  "/etc/rc.stop"
#define TERM     "Linux"
#define TTY      "/dev/tty1"

void rc(char *);
void shutdown(int);
void spawntty(void);
void usage(void);

void
rc(char *cmd) {
  if(fork() == 0) {
    execlp(cmd, cmd, nil);
    alert("can't exec %s: %m", cmd);
    _exit(1);
  }
  wait(nil);
}

void
shutdown(int signum) {
  sync();
  rc(RC_STOP);
  reboot(signum == SIGINT ? RB_AUTOBOOT : RB_POWER_OFF);
}

void
spawntty(void) {
  struct sigaction act;
  int fd;

  if(fork()) {
    wait(nil);
    return;
  }
  act.sa_flags = 0;
  act.sa_handler = SIG_DFL;
  sigfillset(&act.sa_mask);
  sigaction(0, &act, nil);
  setsid();
  chown(TTY, 0, 0);
  chmod(TTY, 0600);
  fd = open(TTY, O_RDWR|O_TTY_INIT);
  if(fd < 0) {
    alert("can't open %s: %m", TTY);
    _exit(1);
  }
  dup2(fd, 0);
  dup2(fd, 1);
  dup2(fd, 2);
  if(fd > 2)
    close(fd);
  setenv("TERM", TERM, 1);
  execl(LOGIN, LOGIN, nil);
  alert("can't exec %s: %m", LOGIN);
  _exit(1);
}

void
usage(void) {
  fprint(stderr, "usage: init\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  struct sigaction act;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND
  
  if(argc > 0)
    usage();
  if(getuid() || getpid() != 1)
    fatal(1, "permission denied");
  act.sa_flags = 0;
  act.sa_handler = SIG_IGN;
  sigfillset(&act.sa_mask);
  sigaction(0, &act, nil);
  signal(SIGINT, shutdown);
  signal(SIGTERM, shutdown);
  reboot(RB_DISABLE_CAD);
  setenv("PATH", PATH, 1);
  rc(RC_START);
  for(;;) {
    spawntty();
    sleep(3);
  }
}
