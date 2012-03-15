#include <avian.h>

int rval;

void
cat(FILE *in, char *s) {
  char buf[8192];
  size_t n;

  while((n = fread(buf, 1, sizeof buf, in)) > 0)
    if(fwrite(buf, 1, n, stdout) != n) {
      alert("write error copying %s: %m", s);
      rval = 1;
      return;
    }
  if(ferror(in)) {
    alert("error reading %s: %m", s);
    rval = 1;
  }
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  ARGBEGIN("u"){
  case 'u':
    setbuf(stdout, nil);
    break;
  default:
    fprint(stderr, "usage: cat [-u] [file...]\n");
    exit(1);
  }ARGEND 
  
  rval = 0;
  if(argc == 0)
    cat(stdin, "<stdin>");
  for(i = 0; i < argc; i++)
    if(!strcmp(argv[i], "-")) {
      cat(stdin, "<stdin>");
    } else {
      f = fopen(argv[i], "r");
      if(f == nil) {
        alert("can't open %s: %m", argv[i]);
        rval = 1;
        continue;
      }
      cat(f, argv[i]);
      fclose(f);
    }
  exit(rval);
}
