#include <avian.h>

int rval;

void
head(FILE *in, char *s, int lines) {
  char *buf;

  while(lines > 0 && (buf = fgetln(in))) {
    println(buf);
    free(buf);
    lines--;
  }
  if(ferror(in)) {
    alert("error reading %s: %m", s);
    rval = 1;
  }
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i, n;

  n = 10;
  ARGBEGIN("n:"){
  case 'n':
    n = atoi(optarg);
    break;
  default:
    fprint(stderr, "usage: head [-n number] [file...]\n");
    exit(1);
  }ARGEND 
  
  rval = 0;
  if(argc == 0)
    head(stdin, "<stdin>", n);
  for(i = 0; i < argc; i++) {
    f = fopen(argv[i], "r");
    if(f == nil) {
      alert("can't open %s: %m", argv[i]);
      rval = 1;
      continue;
    }
    if(argc > 1)
      printf("%c==> %s <==\n", i ? '\n' : 0, argv[i]);
    head(f, argv[i], n);
    fclose(f);
  }
  exit(rval);
}
