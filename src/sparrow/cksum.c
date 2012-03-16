#include <u.h>
#include <avian.h>

int rval;

void
sum(FILE *in, char *s) {
  uchar buf[8192];
  uint32 digest;
  uint n, oct;

  digest = oct = 0;
  while((n = fread(buf, 1, sizeof buf, in)) > 0) {
    digest = crc32(buf, n, digest);
    oct += n;
  }
  if(ferror(in)) {
    alert("error reading %s: %m", s);
    rval = 1;
    return;
  }
  if(in == stdin)
    printf("%lu %zd\n", ~digest, oct);
  else
    printf("%lu %zd %s\n", ~digest, oct, s);
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  ARGBEGIN(""){
  default:
    fprint(stderr, "usage: cksum [file...]\n");
    exit(1);
  }ARGEND
  
  rval = 0;
  if(argc == 0)
    sum(stdin, "<stdin>");
  for(i = 0; i < argc; i++) {
    f = fopen(argv[i], "r");
    if(f == nil) {
      alert("can't open %s: %m", argv[i]);
      rval = 1;
      continue;
    }
    sum(f, argv[i]);
    fclose(f);
  }
  exit(rval);
}
