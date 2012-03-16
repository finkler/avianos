#include <u.h>
#include <avian.h>
#include <sys/mount.h>

void
usage(void) {
  fprint(stderr, "usage: umount [-dfLm] dir\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int flags;

  flags = 0;
  ARGBEGIN("dfLm"){
  case 'd':
    flags |= MNT_DETACH;
    break;
  case 'f':
    flags |= MNT_FORCE;
    break;
  case 'L':
    flags |= UMOUNT_NOFOLLOW;
    break;
  case 'm':
    flags |= MNT_EXPIRE;
    break;
  default:
    usage();
  }ARGEND
  
  if(argc != 1)
    usage();
  if(umount2(argv[0], flags))
    fatal(1, "can't unmount %s: %m", argv[0]);
}
