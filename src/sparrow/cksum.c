#include <u.h>
#include <avian.h>
#include <hash.h>

void
sum(FILE *in, char *s)
{
  uchar buf[8192];
  uint32 digest;
  uint n, total;

  digest = total = 0;
  while((n = fread(buf, 1, sizeof buf, in)) > 0){
    digest += crc32(buf, n, ~digest);
    total += n;
  }
  if(ferror(in)){
    alert("read %s: %m", s);
    return;
  }
  if(in == stdin)
    printf("%u %u\n", ~digest, total);
  else
    printf("%u %u %s\n", ~digest, total, s);
}

int
main(int argc, char *argv[])
{
  FILE *f;
  int i;

  ARGBEGIN(""){
  default:
    fprint(stderr, "usage: cksum [file...]\n");
    exit(1);
  }ARGEND

  if(argc == 0)
    sum(stdin, "<stdin>");
  for(i = 0; i < argc; i++){
    f = fopen(argv[i], "r");
    if(f == nil){
      alert("open %s: %m", argv[i]);
      continue;
    }
    sum(f, argv[i]);
    fclose(f);
  }
  exit(rval);
}
