#include <u.h>
#include <avian.h>

int  flag1;
int  flag2;
int  flag3;
char *tab[] = {"", "\t", "\t\t"};

FILE*
openfile(char *name)
{
  FILE *f;

  if(!strcmp(name, "-"))
    return stdin;
  f = fopen(name, "r");
  if(f == nil)
    fatal(1, "open %s: %m", name);
  return f;
}

void
printcol(int c, char *s)
{
  if(flag1)
    c--;
  if(flag2 && c > 0)
    c--;
  printf("%s%s\n", tab[c], s);
}

void
usage(void)
{
  fprint(stderr, "usage: comm [-123] file1 file2\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  FILE *f1, *f2;
  int m;
  char *p, *q;

  flag1 = flag2 = flag3 = 0;
  ARGBEGIN("123"){
  case '1':
    flag1++;
    break;
  case '2':
    flag2++;
    break;
  case '3':
    flag3++;
    break;
  default:
    usage();
  }ARGEND

  if(argc != 2)
    usage();
  f1 = openfile(argv[0]);
  f2 = openfile(argv[1]);
  m = 0;
  q = nil;
  for(;;){
    p = fgetln(f1);
    if(p)
      p = strdup(p);
    for(;;){
      if(m >= 0){
        free(q);
        q = fgetln(f2);
        if(q)
          q = strdup(q);
      }
      if(p == nil && q == nil)
        goto End;
      if(p && q == nil)
        m = -1;
      else if(p == nil && q)
        m = 1;
      else
        m = strcoll(p, q);
      if(m <= 0)
        break;
      if(!flag2)
        printcol(1, q);
    }
    if(m == 0 && !flag3)
      printcol(2, p);
    if(m < 0 && !flag1)
      printcol(0, p);
    free(p);
  }
End:
  if(ferror(f1) || ferror(f2))
    fatal(1, "read %s: %m", f1 ? argv[0] : argv[1]);
  exit(0);
}
