#include <u.h>
#include <avian.h>
#include <signal.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/un.h>

#define ERR_FILE   "/var/log/errors"
#define KMSG_IFILE "/proc/kmsg"
#define KMSG_OFILE "/var/log/kernel"
#define MSG_FILE   "/var/log/messages"
#define MSG_LEN    512

void logkmsg(void);
void run(void);
void srvmessage(char *);
void usage(void);

FILE *err, *msg;

void
logkmsg(void) {
  char buf[MSG_LEN+1], c;
  FILE *in, *out;
  int n, st;

  if(fork())
    return;
  in = fopen(KMSG_IFILE, "r");
  if(in == nil)
    _exit(1);
  out = fopen(KMSG_OFILE, "w");
  if(out == nil)
    _exit(1);
  for(;;) {
    n = st = 0;
    while((c = fgetc(in)) != EOF && n < MSG_LEN) {
      if(st == 0) {
        if(c == '>')
          st = 1;
        continue;
      }
      buf[n++] = c;
      if(c == '\n')
        break;
    }
    buf[n] = '\0';
    if(fprint(out, buf) == EOF)
      break;
  }
  if(ferror(in))
    alert("error reading %s: %m", KMSG_IFILE);
  _exit(1);
}

void
run(void) {
  char buf[MSG_LEN+1];
  int fd, n;
  struct sockaddr_un in;
  uint len;

  fd = socket(PF_UNIX, SOCK_DGRAM, 0);
  if(fd < 0)
    fatal(1, "can't open socket: %m");
  in.sun_family = AF_UNIX;
  strcpy(in.sun_path, "/dev/log");
  len = sizeof in.sun_family + strlen(in.sun_path);
  if(bind(fd, (struct sockaddr *)&in, len))
    fatal(1, "can't connect socket /dev/log: %m");
  while((n = recvfrom(fd, buf, MSG_LEN, 0, nil, nil)) > 0) {
    if(buf[n-1] == '\n')
      n--;
    buf[n] = '\0';
    srvmessage(buf);
  }
  fatal(1, "error reading /dev/log: %m");
}

void
srvmessage(char *buf) {
  uint prio;
  char *p;

  p = strchr(buf, '>');
  if(p == nil || sscanf(buf, "<%u>", &prio) != 1)
    return;
  p++;
  switch(LOG_PRI(prio)) {
  case LOG_ERR:
    fprintf(err, "*** %s\n", p);
    break;
  case LOG_WARNING:
    fprintf(err, "!!! %s\n", p);
    break;
  case LOG_NOTICE:
    fprintf(msg, "%s\n", p);
    break;
  }
  fflush(err);
  fflush(msg);
}

void
usage(void) {
  fprint(stderr, "usage: syslogd [-k]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int kflag;

  kflag = 0;
  ARGBEGIN("k"){
  case 'k':
    kflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  
  if(argc > 0)
    usage();
  if(getuid())
    fatal(1, "permission denied");
  if(forkdaemon())
    fatal(1, "can't run process: %m");
  if(!kflag)
    logkmsg();
  err = fopen(ERR_FILE, "a");
  if(err == nil)
    fatal(1, "can't open %s: %m", ERR_FILE);
  msg = fopen(MSG_FILE, "a");
  if(msg == nil)
    fatal(1, "can't open %s: %m", MSG_FILE);
  run();
}
