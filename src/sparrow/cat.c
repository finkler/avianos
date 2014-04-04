#include <u.h>
#include <avian.h>

void
cat(FILE *in, char *s)
{
  char buf[8192];
  uint n;

  while((n = fread(buf, 1, sizeof buf, in)) > 0)
    if(fwrite(buf, 1, n, stdout) != n){
      alert("write <stdout>: %m");
      return;
    }
  if(ferror(in))
    alert("read %s: %m", s);
}

int
main(int argc, char *argv[])
{
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

  if(argc == 0)
    cat(stdin, "<stdin>");
  for(i = 0; i < argc; i++)
    if(!strcmp(argv[i], "-"))
      cat(stdin, "<stdin>");
    else{
      f = fopen(argv[i], "r");
      if(f == nil){
        alert("open %s: %m", argv[i]);
        continue;
      }
      cat(f, argv[i]);
      fclose(f);
    }
  exit(rval);
}
