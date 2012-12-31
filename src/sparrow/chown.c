#include <u.h>
#include <avian.h>
#include <grp.h>
#include <ftw.h>
#include <pwd.h>

int gid, uid;

void
usage(void) {
  fprint(stderr, "usage: chown [-h] owner[:group] file...\n"
    "       chown -R [-H|-L|-P] owner[:group] file...\n");
  exit(1);
}

int
walk(const char *fpath, const struct stat *sb, int tflag,
  struct FTW *ftwbuf) {
  if(chown(fpath, uid, gid))
    alert("%s: %m", fpath);
  return 0;
}

int
main(int argc, char *argv[]) {
  char buf[PATH_MAX], *p;
  int flags, i, Rflag, hflag;
  struct group *grp;
  struct passwd *pwd;

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
  gid = -1;
  p = strchr(argv[0], ':');
  if(p != nil) {
    *p++ = '\0';
    if(isdigit(*p))
      grp = getgrgid(atoi(p));
    else
      grp = getgrnam(p);
    if(grp == nil)
      fatal(1, "unknown group %s", p);
    gid = grp->gr_gid;
  }
  if(isdigit(argv[0][0]))
    pwd = getpwuid(atoi(argv[0]));
  else
    pwd = getpwnam(argv[0]);
  if(pwd == nil)
      fatal(1, "unknown user %s", argv[0]);
  uid = pwd->pw_uid;
  for(i = 1; i < argc; i++)
    if(Rflag) {
      if(nftw(argv[i], walk, 20, flags) < 0)
        fatal(1, "%s: %m", argv[i]);
    } else {
      p = argv[i];
      if(!hflag && readlink(p, buf, PATH_MAX) > 0)
        p = buf;
      if(chown(p, uid, gid))
        alert("%s: %m", argv[i]);
    }
  exit(rval);
}
