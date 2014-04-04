#include <u.h>
#include <avian.h>
#include <libgen.h>
#include <sys/stat.h>

int Lflag;
int Pflag;
int fflag;
int sflag;

void
ln(char *s1, char *s2)
{
  struct stat sb;

  if(lstat(s1, &sb)){
    alert("stat %s: %m", s1);
    return;
  }
  if(fflag && access(s2, F_OK) == 0 && unlink(s2)){
    alert("unlink %s: %m", s2);
    return;
  }
  if(S_ISLNK(sb.st_mode)){
    if((Pflag || Lflag)
    && linkat(AT_FDCWD, s1, AT_FDCWD, s2,
      Pflag ? 0 : AT_SYMLINK_FOLLOW))
        alert("linkat: %m");
    return;
  }
  if(sflag){
    if(symlink(s1,s2))
      alert("symlink: %m");
  }else if(link(s1,s2))
    alert("link: %m");
}

void
usage(void)
{
  fprint(stderr, "usage: ln [-fs] [-L|-P] source_file target_file\n"
    "       ln [-fs] [-L|-P] source_file... target_dir\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int i;
  char *p;
  struct stat sb;

  Lflag = Pflag = fflag = sflag = 0;
  ARGBEGIN("LPfs"){
  case 'L':
    Lflag = 1;
    break;
  case 'P':
    Pflag = 1;
    break;
  case 'f':
    fflag = 1;
    break;
  case 's':
    sflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc < 2)
    usage();
  if(sflag)
    Lflag = Pflag = 0;
  if(!stat(argv[argc-1], &sb) && S_ISDIR(sb.st_mode))
    for(i = 0; i < argc; i++){
      p = stradd(argv[argc-1], "/", basename(cleanname(argv[i])));
      ln(argv[i], p);
      free(p);
    }
  else if(argc == 2)
    ln(argv[0], argv[1]);
  else
    fatal(1, "%s: not a directoy", argv[argc-1]);
  exit(rval);
}
