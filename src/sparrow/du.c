#include <avian.h>
#include <ftw.h>

int aflag, blksiz, sflag;
ulong total, subtotal;

void
usage(void) {
  fprint("usage: du [-a|-s] [-kx] [-H|-L] [file...]\n", stderr);
  exit(1);
}

int
walk(const char *fpath, const struct stat *sb, int tflag,
  struct FTW *ftwbuf) {
  ulong siz;

  switch(tflag) {
  case FTW_DP:
    if(!sflag && ftwbuf->level) {
      siz = (sb->st_size + subtotal + (blksiz - 1)) / blksiz;
      printf("%-7lu %s\n", siz, fpath);
    }
    subtotal = 0;
    total += sb->st_size;
    if(!ftwbuf->level) {
      siz = (total + (blksiz - 1)) / blksiz;
      printf("%-7lu %s\n", siz, fpath);
      total = 0;
    }
    break;
  case FTW_SL:
  case FTW_F:
    if(aflag || !ftwbuf->level) {
      siz = (sb->st_size + (blksiz - 1)) / blksiz;
      printf("%-7lu %s\n", siz, fpath);
    }
    subtotal += sb->st_size;
    total += sb->st_size;
    break;
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  int flags, i;

  aflag = sflag = 0;
  blksiz = 512;
  flags = FTW_DEPTH | FTW_PHYS;
  ARGBEGIN("HLaksx"){
  case 'H':
    break;
  case 'L':
    flags &= ~FTW_PHYS;
    break;
  case 'a':
    aflag = 1;
    break;
  case 'k':
    blksiz = 1024;
    break;
  case 's':
    sflag = 1;
    break;
  case 'x':
    flags |= FTW_MOUNT;
    break;
  default:
    usage();
  }ARGEND 
  
  if(aflag+sflag > 1)
    usage();
  subtotal = total = 0;
  if(argc == 0)
    if(nftw(".", walk, 20, flags))
      fatal(1, ".: %m");
  for(i = 0; i < argc; i++)
    if(nftw(argv[i], walk, 20, flags))
      fatal(1, "%s: %m", argv[i]);
}
