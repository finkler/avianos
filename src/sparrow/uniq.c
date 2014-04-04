#include <u.h>
#include <avian.h>

int num;
int off;

char*
skip(char *s)
{
  uint n;
  int i;

  n = strlen(s);
  if(n > 0 && s[n-1] == '\n')
    s[--n] = '\0';
  for(i = 0; i < num && *s != '\0'; i++){
    while(isblank(*s))
      s++;
    while(*s != '\0' && !isblank(*s))
      s++;
  }
  for(i = 0; i < off && *s != '\0'; i++)
    s++;
  return s;
}

void
usage(void)
{
  fprint(stderr, "usage: uniq [-c|-d|-u] [-f fields] [-s char] "
    "[input_file [output_file]]\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  char buf[2][LINE_MAX+1];
  int cflag, dflag, uflag;
  int n;

  for(n = 1; n < argc && strcmp(argv[n], "--"); n++)
    if(argv[n][0] == '+')
      argv[n][0] = '-';
  cflag = dflag = uflag = 0;
  num = off = 0;
  ARGBEGIN("cdf:s:u"){
  case 'c':
    cflag = 1;
    break;
  case 'd':
    dflag = 1;
    break;
  case 'f':
    num = atoi(optarg);
    break;
  case 's':
    off = atoi(optarg);
    break;
  case 'u':
    uflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(cflag+dflag+uflag > 1 || argc > 2)
    usage();
  if(argc > 1 && !freopen(argv[1], "w", stdout))
    fatal(1, "open %s: %m", argv[1]);
  if(argc > 0 && strcmp(argv[0], "-"))
    if(!freopen(argv[0], "r", stdin))
      fatal(1, "open %s: %m", argv[0]);
  while(fgets(buf[0], LINE_MAX, stdin)){
    for(n = 1; fgets(buf[1], LINE_MAX, stdin); n++)
      if(strcmp(skip(buf[0]), skip(buf[1])))
        break;
    if(n == 1 && dflag)
      continue;
    if(!(n > 1 && uflag)){
      if(cflag)
        printf("%4d ", n);
      println(buf[0]);
    }
    if(!(n > 1 && dflag)){
      if(cflag)
        print("    1 ");
      println(buf[1]);
    }
  }
}
