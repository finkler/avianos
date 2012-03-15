#include <avian.h>
#include <hash.h>

int rval;

void
sum(FILE *in, char *s) {
  uchar buf[8192];
  char *p;
  int n;
  SHA1Digest *digest;

  digest = nil;
  while((n = fread(buf, 1, sizeof buf, in)) > 0)
    digest = sha1(buf, n, digest);
  if(ferror(in)) {
    alert("error reading %s: %m", s);
    rval = 1;
    return;
  }
  p = sha1pickle(digest);
  if(in == stdin)
    printf("%s\n", p);
  else
    printf("%s %s\n", p, s);
  free(digest);
  free(p);
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  ARGBEGIN(""){
  default:
    fprint("usage: sha1sum [file...]\n", stderr);
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
