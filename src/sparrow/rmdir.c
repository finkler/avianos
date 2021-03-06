#include <u.h>
#include <avian.h>
#include <libgen.h>

int
cmp(const void *v1, const void *v2)
{
  return -(strcmp(*(char*const*)v1, *(char*const*)v2));
}

void
usage(void)
{
  fprint(stderr, "usage: rmdir [-p] dir...\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int i, pflag;
  char *p;

  pflag = 0;
  ARGBEGIN("p"){
  case 'p':
    pflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc < 1)
    usage();
  qsort(argv, argc, sizeof(char*), cmp);
  for(i = 0; i < argc; i++)
    for(p = argv[i]; strcmp(p, "."); p = dirname(p)){
      if(rmdir(p))
        alert("rmdir %s: %m", p);
      if(!pflag)
        break;
    }
  exit(rval);
}
