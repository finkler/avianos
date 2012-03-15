#include <avian.h>
#include <errno.h>
#include <sys/stat.h>

int
mkpath(char *path, mode_t mode) {
  char buf[PATH_MAX];
  int i;

  if(strlen(path) >= PATH_MAX) {
    errno = ENAMETOOLONG;
    return -1;
  }
  i = 0;
  if(*path == '/')
    buf[i++] = *path++;
  for(;;) {
    while(*path && *path != '/')
      buf[i++] = *path++;
    buf[i] = '\0';
    if(access(buf, F_OK) && mkdir(buf, mode))
      return -1;
    if(*path == '\0')
      break;
    buf[i++] = *path++;
  }
  return 0;
}

void
usage(void) {
  fprint("usage: mkdir [-p] [-m mode] dir...\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int i, pflag, rval;
  mode_t m;

  m = S_IRWXU|S_IRWXG|S_IRWXO;
  pflag = 0;
  ARGBEGIN("m:p"){
  case 'm':
    m = getmode(optarg, m);
    break;
  case 'p':
    pflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  if(argc < 1)
    usage();
  rval = 0;
  for(i = 0; i < argc; i++)
    if(pflag ? mkpath(argv[i], m)
      : mkdir(argv[i], m)) {
      alert("%s: %m", argv[i]);
      rval = 1;
    }
  exit(rval);
}
