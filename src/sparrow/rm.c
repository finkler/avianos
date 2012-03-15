#include <avian.h>
#include <errno.h>
#include <ftw.h>
#include <libgen.h>
#include <sys/stat.h>

int fflag, iflag;
int rval;

int
ask(const char *s) {
  char *ans;

  if(!fflag && ((access(s, W_OK) && isatty(0)) || iflag)) {
    fprintf(stderr, "rm: remove %s?[n]: ", s);
    ans = fgetln(stdin);
    if(*ans != 'y' && *ans != 'Y') {
      free(ans);
      return 0;
    }
    free(ans);
  }
  return 1;
}

int
rmtree(const char *fpath, const struct stat *sb, int tflag,
  struct FTW *ftwbuf) {
  switch (tflag) {
  case FTW_DP:
    return(ask(fpath) && rmdir(fpath));
  case FTW_F:
  case FTW_SL:
    return(ask(fpath) && unlink(fpath));
  }
  return 0;
}

void
usage(void) {
  fprint(stderr, "usage: rm [-fiRr] file...\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int i, rflag;
  char *p;
  struct stat sb;

  fflag = iflag = rflag = 0;
  ARGBEGIN("Rfir"){
  case 'f':
    fflag = 1;
    iflag = 0;
    break;
  case 'i':
    fflag = 0;
    iflag = 1;
    break;
  case 'R':
  case 'r':
    rflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  
  if(argc < 1)
    usage();
  rval = 0;
  for(i = 0; i < argc; i++) {
    p = basename(cleanname(argv[i]));
    if(!strcmp(p, ".") || !strcmp(p, ".."))
      goto failed;
    if(lstat(argv[i], &sb))
      goto failed;
    if(S_ISDIR(sb.st_mode)) {
      if(!rflag) {
        errno = EISDIR;
        goto failed;
      }
      if(nftw(argv[i], rmtree, 20, FTW_DEPTH | FTW_PHYS))
        goto failed;
    } else if(ask(argv[i]) && unlink(argv[i]))
        goto failed;
    continue;
  failed:
    if(!fflag)
      alert(errno ? "can't remove %s: %m" :
        "can't remove %s", argv[i]);
    rval = 1;
    errno = 0;
  }
  exit(rval);
}
