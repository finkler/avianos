#include <u.h>
#include <avian.h>
#include <libgen.h>

void
usage(void)
{
  fprint(stderr, "usage: basename string [suffix]\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  uint n;
  char *p;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND

  if(argc != 1 && argc != 2)
    usage();
  p = basename(argv[0]);
  if(argc == 2){
    n = strlen(p)-strlen(argv[1]);
    if(n > 0 && !strcmp(p+n, argv[1]))
      p[n] = '\0';
  }
  println(p);
}
