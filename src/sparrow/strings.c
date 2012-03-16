#include <u.h>
#include <avian.h>

char fmt[4];
int len, rval;

void
strings(FILE *in, char *s) {
  char buf[LINE_MAX];
  int c, i, n;

  i = 0;
  for(n = 1; (c = fgetc(in)) != EOF; n++)
    if(isprint(c) && i < LINE_MAX) {
      buf[i++] = c;
    } else {
      if(i >= len && (c == 0 || c == '\n')) {
        if(*fmt)
          printf(fmt, n);
        buf[i] = '\0';
        println(buf);
      }
      i = 0;
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

  *fmt = '\0';
  len = 4;
  ARGBEGIN("an:t:"){
  case 'a':
    break;
  case 'n':
    len = atoi(optarg);
    break;
  case 't':
    if(strlen(optarg) != 1 || !strpbrk(optarg, "dox"))
      fatal(1, "illegal format %s", optarg);
    sprintf(fmt, "%%%c ", *optarg);
    break;
  default:
    fprint(stderr,
      "usage: strings [-a] [-t format] [-n number]  [file...]\n");
    exit(1);
  }ARGEND 
  
  rval = 0;
  if(argc == 0)
    strings(stdin, "<stdin>");
  for(i = 0; i < argc; i++) {
    f = fopen(argv[i], "r");
    if(f == nil) {
      alert("can't open %s: %m", argv[i]);
      rval = 1;
      continue;
    }
    strings(f, argv[i]);
    fclose(f);
  }
  exit(rval);
}
