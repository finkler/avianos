#include <u.h>
#include <avian.h>
#include <sys/stat.h>
#include <time.h>

#define MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

struct timespec times[2];

void
datetime(char *s) {
  char *p;
  struct tm tm;

  p = strchr(s, ' ');
  if(p != nil)
    *p = 'T';
  p = strptime(s, "%Y-%m-%dT%H:%M", &tm);
  if(p == nil)
    fatal(1, "%s: invalid date format", s);
  if(*p == '.' || *p == ',')
    times[0].tv_nsec = times[1].tv_nsec =
      strtol(++p, &p, 10)%(long)10e9;
  if(*p == 'Z') {
    p++;
    setenv("TZ", "UTC-0", 1);
  }
  if(*p != '\0')
    fatal(1, "%s: invalid date format", s);
  times[0].tv_sec = times[1].tv_sec = mktime(&tm);
}

void
reftime(char *path) {
  struct stat sb;

  if(stat(path, &sb))
    fatal(1, "stat %s: %m", path);
  times[0] = sb.st_atim;
  times[1] = sb.st_mtim;
}

void
timetime(char *s) {
  char *fmt;
  uint n;
  struct tm tm;

  fmt = nil;
  n = 0;
  switch(strlen(s)) {
  case 12:
    n = 10;
  case 15:
    fmt = "%Y%m%d%H%M.%S";
    break;
  case 10:
    n = 10;
  case 13:
    fmt = "%y%m%d%H%M.%S";
    break;
  case 8:
    n = 8;
  case 11:
    fmt = "%m%d%H%M.%S";
    break;
  }
  if(fmt && n)
    fmt[n] = '\0';
  if(fmt == nil || strptime(s, fmt, &tm) == nil)
    fatal(1, "%s: invalid time format", s);
  times[0].tv_sec = times[1].tv_sec = mktime(&tm);
}

void
usage(void) {
  fprint(stderr, "usage: touch [-acm] "
    "[-r ref_file|-t time|-d date_time] file...\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int aflag, cflag, mflag;
  int fd, i, timeset;

  aflag = cflag = mflag = 0;
  timeset = 0;
  ARGBEGIN("acd:mr:t:"){
  case 'a':
    aflag = 1;
    break;
  case 'c':
    cflag = 1;
    break;
  case 'd':
    datetime(optarg);
    timeset = 1;
    break;
  case 'm':
    mflag = 1;
    break;
  case 'r':
    reftime(optarg);
    timeset = 1;
    break;
  case 't':
    timetime(optarg);
    timeset = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc < 1)
    usage();
  if(!(aflag+mflag))
    aflag = mflag = 1;
  if(!timeset)
    times[0].tv_nsec = times[1].tv_nsec = UTIME_NOW;
  if(!aflag)
    times[0].tv_nsec = UTIME_OMIT;
  if(!mflag)
    times[1].tv_nsec = UTIME_OMIT;
  for(i = 0; i < argc; i++)
    if(!access(argv[i], F_OK)) {
      if(utimensat(AT_FDCWD, argv[i], times, 0))
        alert("%s: %m", argv[i]);
    } else if(!cflag) {
      fd = creat(argv[i], MODE);
      if(fd < 0) {
        alert("can't create %s: %m", argv[i]);
        continue;
      }
      if(futimens(fd, times))
        alert("%s: %m", argv[i]);
      close(fd);
    }
  exit(rval);
}
