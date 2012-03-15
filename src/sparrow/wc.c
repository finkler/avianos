#include <avian.h>

int cflag, lflag, wflag;
int rval;
int totb, totw, totl;

void
wc(FILE *in, char *s) {
  int byte, word, line;
  int c, p;

  byte = line = word = 0;
  for(p = ' '; (c = fgetc(in)) != EOF; p = c) {
    byte++;
    if(isspace(c)) {
      if(c == '\n')
        line++;
      if(!isspace(p))
        word++;
    }
  }
  if(ferror(in)) {
    alert("%s: %m", s);
    rval = 1;
  }
  totb += byte;
  totl += line;
  totw += word;
  if(lflag)
    printf(" %7d", line);
  if(wflag)
    printf(" %7d", word);
  if(cflag)
    printf(" %7d", byte);
  if(in != stdin)
    printf(" %s", s);
  print('\n');
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  cflag = lflag = wflag = 0;
  ARGBEGIN("clmw"){
  case 'c':
  case 'm':
    cflag = 1;
    break;
  case 'l':
    lflag = 1;
    break;
  case 'w':
    wflag = 1;
    break;
  default:
    fprint("usage: wc [-c|-m] [-lw] [file...]\n", stderr);
    exit(1);
  }ARGEND 
  if(!(cflag + lflag + wflag))
    cflag = lflag = wflag = 1;
  rval = 0;
  if(argc == 0)
    wc(stdin, "<stdin>");
  for(i = 0; i < argc; i++) {
    if(!(f = fopen(argv[i], "r"))) {
      alert("can't open %s: %m", argv[i]);
      rval = 1;
      continue;
    }
    wc(f, argv[i]);
    fclose(f);
  }
  if(argc > 1) {
    if(lflag)
      printf(" %7d", totl);
    if(wflag)
      printf(" %7d", totw);
    if(cflag)
      printf(" %7d", totb);
    println(" total");
  }
  exit(rval);
}
