#include <u.h>
#include <avian.h>
#include <regex.h>

typedef struct Pattern Pattern;
struct Pattern {
  union {
    char *fix;
    regex_t reg;
  } exp;
  Pattern *next;
};

int Fflag, cflag, lflag, qflag;
int nflag, sflag, vflag, xflag;
int flags;
Pattern *c;

void
addpattern(char *s) {
  Pattern *new;

  new = malloc(sizeof(Pattern));
  if(Fflag)
    new->exp.fix = strdup(s);
  else if(regcomp(&new->exp.reg, s, flags))
    fatal(2, "invalid pattern %s", s);
  new->next = c;
  c = new;
}

void
grep(FILE *in, char *s, int pname) {
  char *buf;
  int i, j;
  Pattern *r;

  j = 0;
  for(i = 1; (buf = fgetln(in)); i++) {
    for(r = c; r; r = r->next)
      if((Fflag && ((xflag && !strcmp(buf, r->exp.fix)) ||
        strstr(buf, r->exp.fix))) ||
        !regexec(&r->exp.reg, buf, 0, nil, 0))
        break;
    if((vflag && r) || (!vflag && !r))
      continue;
    if(rval == 0)
      rval = 1;
    if(cflag) {
      j++;
      continue;
    }
    if(lflag) {
      println(s);
      break;
    }
    if(qflag)
      exit(0);
    if(pname)
      printf("%s:", s);
    if(nflag)
      printf("%d:", i);
    println(buf);
  }
  if(ferror(in)) {
    if(!sflag)
      alert("read %s: %m", s);
    rval = 2;
    return;
  }
  if(cflag) {
    if(pname)
      printf("%s:", s);
    printf("%d\n", j);
  }
}

void
parsefile(char *path) {
  char *buf;
  FILE *f;

  f = fopen(path, "r");
  if(f == nil)
    fatal(2, "can't open %s: %m", path);
  while((buf = fgetln(f)))
    addpattern(buf);
  fclose(f);
}

void
parselist(char *s) {
  uint n;
  char *p;

  if(s[0] == '\n')
    s++;
  n = strlen(s);
  if(n > 0 && s[n-1] == '\n')
    s[n-1] = '\0';
  while((p = strrchr(s, '\n'))) {
    *p++ = '\0';
    addpattern(p);
  }
  addpattern(s);
}

void
usage(void) {
  fprint(stderr, "usage: grep [-E|-F] [-c|-l|-q] [-insvx] -e pattern_list\n"
    "\t[-e pattern_list]... [-f pattern_file]... [file...]\n"
    "       grep [-E|-F] [-c|-l|-q] [-insvx] [-e pattern_list]...\n"
    "\t-f pattern_file [-f pattern_file]... [file...]\n"
    "       grep [-E|-F] [-c|-l|-q] [-insvx] pattern_list [file...]\n");
  exit(2);
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  c = nil;
  Fflag = cflag = lflag = qflag = 0;
  nflag = sflag = xflag = vflag = 0;
  flags = REG_NOSUB;
  ARGBEGIN("EFce:f:ilnqsvx"){
  case 'E':
    flags |= REG_EXTENDED;
    break;
  case 'F':
    Fflag = 1;
    break;
  case 'c':
    cflag = 1;
    break;
  case 'e':
    parselist(optarg);
    break;
  case 'f':
    parsefile(optarg);
    break;
  case 'i':
    flags |= REG_ICASE;
    break;
  case 'l':
    lflag = 1;
    break;
  case 'n':
    nflag = 1;
    break;
  case 'q':
    qflag = 1;
    break;
  case 's':
    sflag = 1;
    break;
  case 'v':
    vflag = 1;
    break;
  case 'x':
    xflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(cflag+lflag+qflag > 1)
    usage();
  if(c == nil) {
    if(argc < 1)
      usage();
    parselist(argv[0]);
    argc--, argv++;
  }
  if(argc == 0)
    grep(stdin, "<stdin>", 0);
  for(i = 0, rval = 0; i < argc; i++) {
    f = fopen(argv[i], "r");
    if(f == nil) {
      if(!sflag)
        alert("open %s: %m", argv[i]);
      rval = 2;
      continue;
    }
    grep(f, argv[i], argc > 1);
    fclose(f);
  }
  exit(rval);
}
