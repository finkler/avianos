#include <u.h>
#include <avian.h>
#include <sys/utsname.h>

#define SYSNAME  0x01
#define NODENAME 0x02
#define RELEASE  0x04
#define VERSION  0x08
#define MACHINE  0x10
#define ALL      0x1f

void
usage(void) {
  fprint(stderr, "usage: uname [-amnrsv]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int mask, sp;
  struct utsname u;

  mask = 0;
  ARGBEGIN("amnrsv"){
  case 'a':
    mask = ALL;
    break;
  case 'm':
    mask |= MACHINE;
    break;
  case 'n':
    mask |= NODENAME;
    break;
  case 'r':
    mask |= RELEASE;
    break;
  case 's':
    mask |= SYSNAME;
    break;
  case 'v':
    mask |= VERSION;
    break;
  default:
    usage();
  }ARGEND 
  
  if(argc != 0)
    usage();
  if(uname(&u))
    fatal(1, "%m");
  sp = 0;
  if(!mask || mask & SYSNAME) {
    sp++;
    print(u.sysname);
  }
  if(mask & NODENAME) {
    if(sp++)
      print(" ");
    print(u.nodename);
  }
  if(mask & RELEASE) {
    if(sp++)
      print(" ");
    print(u.release);
  }
  if(mask & VERSION) {
    if(sp++)
      print(" ");
    print(u.version);
  }
  if(mask & MACHINE) {
    if(sp++)
      print(" ");
    print(u.machine);
  }
  print("\n");
}
