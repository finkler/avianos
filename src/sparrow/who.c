#include <avian.h>
#include <fcntl.h>
#include <time.h>
#include <utmpx.h>
#include <sys/stat.h>

void
usage(void) {
  fprint("usage: who [-mTu]\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char buf[32], c;
  int fd, mflag, Tflag, uflag;
  time_t ltm;
  char path[PATH_MAX], *tty;
  struct stat sb;
  struct utmpx *utx;

  c = mflag = uflag = Tflag = 0;
  tty = nil;
  ARGBEGIN("mTu"){
  case 'm':
    tty = ttyname(0) + 5;
    mflag = 1;
    break;
  case 'T':
    Tflag = 1;
    break;
  case 'u':
    uflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  if(argc != 0)
    usage();
  setutxent();
  while((utx = getutxent())) {
    if(utx->ut_type != USER_PROCESS || (mflag && strcmp(tty, utx->ut_line)))
      continue;
    ltm = utx->ut_tv.tv_sec;
    strftime(buf, sizeof buf, "%b %e %H:%M", localtime(&ltm));
    if(Tflag || uflag) {
      sprintf(path, "/dev/%s", utx->ut_line);
      if((fd = open(path, O_RDONLY)) < 0 || fstat(fd, &sb))
        c = '?';
      else if(!isatty(fd))
        c = ' ';
      else if(sb.st_mode & S_IWOTH)
        c = '+';
      else c = '-';
    }
    printf("%s\t%c %s\t%s", utx->ut_user, Tflag ? c : 0, utx->ut_line, buf);
    if(uflag)
      printf("\t%ld", c == '?' ? 0 :
        time(nil)-sb.st_atim.tv_sec);
    print('\n');
  }
  endutxent();
}
