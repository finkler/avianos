#include <u.h>
#include <avian.h>
#include <grp.h>
#include <ftw.h>
#include <pwd.h>

int gid;

void
usage(void) {
  fprint(stderr, "usage: chgrp [-h] group file...\n"
    "       chgrp -R [-H|-L|-P] group file...\n");
  exit(1);
}

int
walk(const char *fpath, const struct stat *sb, int tflag,
  struct FTW *ftwbuf) {
  if(chown(fpath, -1, gid))
    alert("%s: %m", fpath);
  return 0;
}

int
main(int argc, char *argv[]) {
  char buf[PATH_MAX], *p;
  int flags, i, Rflag, hflag;
  struct group *grp;

  Rflag = hflag = 0;
  flags = 0;
  ARGBEGIN("hHLPR"){
  case 'h':
    hflag = 1;
    break;
  case 'H':
    break;
  case 'L':
    break;
  case 'P':
    flags = FTW_PHYS;
    break;
  case 'R':
    Rflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  
  if(argc < 2)
    usage();
  rval = 0;
  if(isdigit(argv[0][0]))
    grp = getgrgid(atoi(argv[0]));
  else
    grp = getgrnam(argv[0]);
  if(grp == nil)
    fatal(1, "unknown group %s", argv[0]);
  gid = grp->gr_gid;
  for(i = 1; i < argc; i++)
    if(Rflag) {
      if(nftw(argv[i], walk, 20, flags) < 0)
        fatal(1, "%s: %m", argv[i]);
    } else {
      p = argv[i];
      if(!hflag && readlink(p, buf, PATH_MAX) > 0)
        p = buf;
      if(chown(p, -1, gid))
        alert("%s: %m", argv[i]);
    }
  exit(rval);
}
