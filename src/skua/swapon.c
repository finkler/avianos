#include <u.h>
#include <avian.h>
#include <sys/swap.h>

void
usage(void) {
  fprint(stderr, "usage: swapon [-d] [-p priority] file...\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int flags, i, prio;

  flags = prio = 0;
  ARGBEGIN("dp:"){
  case 'd':
    flags |= SWAP_FLAG_DISCARD;
    break;
  case 'p':
    prio = atoi(optarg);
    flags |= SWAP_FLAG_PREFER|((prio<<SWAP_FLAG_PRIO_SHIFT)&
      SWAP_FLAG_PRIO_MASK);
    break;
  default:
    usage();
  }ARGEND

  if(argc < 1)
    usage();
  for(i = 0; i < argc; i++)
    if(swapon(argv[i], flags))
      alert("swapon: %m");
  exit(rval);
}
