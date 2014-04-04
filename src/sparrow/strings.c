#include <u.h>
#include <avian.h>
#include <linux/elf.h>
#include <utf8.h>

char fmt[4];
int  aflag;
int  tflag;
int  len;

void
skipelf(FILE *f)
{
  long off;

  alert("elf is such a complicated format");
}

void
strings(FILE *in, char *s)
{
  char buf[LINE_MAX+1], magic[4];
  int c, i, n, off;

  i = 0;
  for(n = 1; (c = fgetc(in)) != EOF; n++)
    if(!aflag && n == 1 && c == 0x7f){
      /* assume elf header */
      skipelf(in);
      continue;
    }
    if(isprint(c) && i < LINE_MAX)
      buf[i++] = c;
    else{
      buf[i] = '\0';
      if(utflen(buf) >= len && (c == 0 || c == '\n')){
        if(tflag)
          printf(fmt, n);
        println(buf);
      }
      i = 0;
    }
  if(ferror(in))
    alert("read %s: %m", s);
}

int
main(int argc, char *argv[])
{
  FILE *f;
  int i;

  aflag = tflag = 0;
  len = 4;
  ARGBEGIN("an:t:"){
  case 'a':
    aflag = 1;
    break;
  case 'n':
    len = atoi(optarg);
    break;
  case 't':
    if(!strpbrk(optarg, "dox") || optarg[1] != '\0')
      fatal(1, "invalid format %s", optarg);
    sprintf(fmt, "%%%c ", *optarg);
    tflag = 1;
    break;
  default:
    fprint(stderr, "usage: strings [-a] [-t format] [-n number] [file...]\n");
    exit(1);
  }ARGEND

  if(argc == 0)
    strings(stdin, "<stdin>");
  for(i = 0; i < argc; i++){
    f = fopen(argv[i], "r");
    if(f == nil){
      alert("open %s: %m", argv[i]);
      continue;
    }
    strings(f, argv[i]);
    fclose(f);
  }
  exit(rval);
}
