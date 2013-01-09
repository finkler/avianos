#include <u.h>
#include <avian.h>
#include <sys/stat.h>

void
usage(void) {
  fprint(stderr, "usage: mkfifo [-m mode] file...\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int i;
  uint m;

  m = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
  ARGBEGIN("m:"){
  case 'm':
    m = symmod(m, optarg);
    break;
  default:
    usage();
  }ARGEND

  if(argc < 1)
    usage();
  for(i = 0; i < argc; i++)
    if(mkfifo(argv[i], m))
      alert("%s: %m", argv[i]);
  exit(rval);
}
