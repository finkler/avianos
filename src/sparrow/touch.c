#include <avian.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

struct timespec times[2];

void
datetime(char *s) {
  char *p;
  struct tm tm;

  if((p = strchr(s, ' ')))
    *p = 'T';
  p = strptime(s, "%Y-%m-%dT%H:%M", &tm);
  if(p == nil)
    fatal(1, "invalid date format %s", s);
  if(*p == '.' || *p == ',')
    times[0].tv_nsec = times[1].tv_nsec = strtol(++p, &p, 10)%(long)10e9;
  if(*p == 'Z') {
    p++;
    setenv("TZ", "UTC0", 1);
  }
  if(*p != '\0')
    fatal(1, "invalid date format %s", s);
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
  size_t n;
  char *p;
  struct tm tm;

  if((n = strlen(s)) > 15)
    fatal(1, "invalid time format %s", s);
  memset(&tm, 0, sizeof tm);
  if((p = strchr(s, '.'))) {
    tm.tm_sec = atoi(++p);
    n -= 3;
  }
  if(n == 12) {
    sscanf(p, "%4d", &tm.tm_year);
    tm.tm_year -= 1900;
    p += 4;
  } else if(n == 10) {
    sscanf(p, "%2d", &tm.tm_year);
    if(tm.tm_year < 69)
      tm.tm_year += 100;
    p += 2;
  } else if(n != 8)
    fatal(1, "invalid time format %s", s);
  sscanf(p, "%2d%2d%2d%2d", &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min);
  tm.tm_mon--;
  times[0].tv_sec = times[1].tv_sec = mktime(&tm);
}

void
usage(void) {
  fprint("usage: touch [-acm] [-r ref_file|-t time|-d date_time] file...\n",
    stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int aflag, cflag, mflag;
  int fd, i, rval, timeset;

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
  rval = 0;
  for(i = 0; i < argc; i++)
    if(!access(argv[i], F_OK)) {
      if(utimensat(AT_FDCWD, argv[i], times, 0)) {
        alert("%s: %m", argv[i]);
        rval = 1;
      }
    } else if(!cflag) {
      if((fd = creat(argv[i], MODE)) < 0) {
        alert("can't create %s :%m", argv[i]);
        rval = 1;
        continue;
      }
      if(futimens(fd, times)) {
        alert("%s: %m", argv[i]);
        rval = 1;
      }
      close(fd);
    }
  exit(rval);
}
