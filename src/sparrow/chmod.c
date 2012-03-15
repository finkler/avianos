#include <avian.h>
#include <ftw.h>
#include <sys/stat.h>

char *modstr;
int rval;

void
usage(void) {
  fprint(stderr, "usage: chmod [-R] mode file...\n");
  exit(1);
}

int
walk(const char *fpath, const struct stat *sb, int tflag,
  struct FTW *ftwbuf) {
  if(chmod(fpath, getmode(modstr, sb->st_mode))) {
    alert("%s: %m", fpath);
    rval = 1;
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  char buf[PATH_MAX], *p;
  int i, Rflag;
  struct stat sb;

  Rflag = 0;
  ARGBEGIN("R"){
  case 'R':
    Rflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  
  if(argc < 2)
    usage();
  modstr = argv[0];
  rval = 0;
  for(i = 1; i < argc; i++)
    if(stat(argv[i], &sb)) {
      alert("can't stat %s: %m", argv[i]);
      rval = 1;
      continue;
    }
    if(S_ISDIR(sb.st_mode) && Rflag) {
      if(nftw(argv[i], walk, 20, 0))
        fatal(1, "%s: %m", argv[i]);
    } else {
      p = argv[i];
      if(readlink(p, buf, PATH_MAX) > 0)
        p = buf;
      if(chmod(p, getmode(modstr, sb.st_mode))) {
        alert("%s: %m", p);
        rval = 1;
      }
    }
  exit(rval);
}