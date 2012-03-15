#include <avian.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>

int Lflag, Pflag, fflag, sflag;
int rval;
struct stat sb;

void
ln(char *s1, char *s2) {
  if(lstat(s1, &sb))
    goto failed;
  if(!access(s2, F_OK) && fflag && unlink(s2))
    goto failed;
  if(S_ISLNK(sb.st_mode)) {
    if((Pflag || Lflag) && linkat(AT_FDCWD, s1, AT_FDCWD, s2, Pflag ? 0 : AT_SYMLINK_FOLLOW))
      goto failed;
  } else if(sflag ? symlink(s1, s2) : link(s1, s2))
    goto failed;
  return;
failed:
  alert("%s on %s: %m", s1, s2);
  rval = 1;
}

void
usage(void) {
  fprint("usage: ln [-fs] [-L|-P] source_file target_file\n" "       ln [-fs] [-L|-P] source_file... target_dir\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int i;
  char path[PATH_MAX];

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
  rval = 0;
  if(sflag) {
    Lflag = Pflag = 0;
  if(!stat(argv[argc-1], &sb) && S_ISDIR(sb.st_mode)) {
    cleanname(argv[--argc]);
    for(i = 0; i < argc; i++) {
      snprintf(path, PATH_MAX, "%s/%s", argv[argc], basename(cleanname(argv[i])));
      ln(argv[i], path);
    }
  } else if(argc == 2)
    ln(argv[0], argv[1]);
  } else
    fatal(1, "%s: is not a directoy", argv[argc-1]);
  exit(rval);
}
