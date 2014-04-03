#include <u.h>
#include <avian.h>

#define INCR 10

int  compar(const void*, const void*);
void expand(FILE*, char*);
void tablist(char*);
int  tabstop(int);

int len;
int *tab;

int
compar(const void *a, const void *b)
{
  return *(int*)a - *(int*)b;
}

void
expand(FILE *in, char *s)
{
  int c, n, ts;

  n = 0;
  while((c = fgetc(in)) != EOF){
    if(c == '\t'){
      ts = tabstop(n);
      do{
        fputc(' ', stdout);
        n++;
      }while(ts > 1 && n%ts != 1);
      continue;
    }
    n++;
    fputc(c, stdout);
    if(c == '\n')
      n = 0;
  }
  if(ferror(in))
    alert("read %s: %m", s);
}

void
tablist(char *s)
{
  char *p;
  int cap;

  cap = INCR;
  len = 0;
  tab = malloc(cap*sizeof(int));
  for(p = strtok(s, " ,\t"); p; p = strtok(nil, " ,\t")){
    if(cap == len){
      cap += INCR;
      tab = realloc(tab, cap*sizeof(int));
    }
    tab[len++] = atoi(p);
  }
  qsort(tab, len, sizeof(int), compar);
}

int
tabstop(int n)
{
  int i;

  if(len == 1)
    return tab[0];
  i = 0;
  while(tab[i] < n && i < len)
    i++;
  if(i == len)
    return 0;
  return tab[i];
}

int
main(int argc, char *argv[])
{
  FILE *f;
  int i;

  ARGBEGIN("t:"){
  case 't':
    tablist(optarg);
    break;
  default:
    fprint(stderr, "usage: expand [-t tablist] [file...]\n");
    exit(1);
  }ARGEND

  if(len == 0)
    tablist("8");
  if(argc == 0)
    expand(stdin, "<stdin>");
  for(i = 0; i < argc; i++){
    f = fopen(argv[i], "r");
    if(f == nil){
      alert("open %s: %m", argv[i]);
      continue;
    }
    expand(f, argv[i]);
    fclose(f);
  }
}
