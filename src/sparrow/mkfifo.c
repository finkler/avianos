#include <avian.h>
#include <sys/stat.h>

void
usage(void) {
  fprint("usage: mkfifo [-m mode] file...\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int i, rval;
  mode_t m;

  m = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
  ARGBEGIN("m:"){
  case 'm':
    m = getmode(optarg, m);
    break;
  default:
    usage();
  }ARGEND 
  if(argc < 1)
    usage();
  for(i = 0; i < argc; i++)
    if(mkfifo(argv[i], m)) {
      alert("%s: %m", argv[i]);
      rval = 1;
    }
  exit(rval);
}
