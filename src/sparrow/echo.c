#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[]) {
  int i, nflag;

  nflag = 0;
  if(argc > 1 && !strcmp(argv[1], "-n"))
    nflag = 1;

  for(i = 1+nflag; i < argc; i++) {
    fprint(stdout, argv[i]);
    if(i < argc-1)
      print(" ");
  }
  if(!nflag)
    print("\n");
}
