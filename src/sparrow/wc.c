#include <u.h>
#include <avian.h>
#include <utf8.h>

int cflag, lflag, mflag, wflag;
int totb, totw, totl, totc;

void
wc(FILE *in, char *s) {
  int byte, chr, line, word;
  int c, p;

  byte = chr = line = word = 0;
  for(p = ' '; (c = fgetc(in)) != EOF; p = c) {
    if(c < RUNE_SELF || runestart(c))
      chr++;
    byte++;
    if(isspace(c)) {
      if(c == '\n')
        line++;
      if(!isspace(p))
        word++;
    }
  }
  if(ferror(in))
    alert("%s: %m", s);
  totb += byte;
  totc += chr;
  totl += line;
  totw += word;
  if(lflag)
    printf(" %7d", line);
  if(wflag)
    printf(" %7d", word);
  if(cflag)
    printf(" %7d", byte);
  if(mflag)
    printf(" %7d", chr);
  if(in != stdin)
    printf(" %s", s);
  print("\n");
}

void
usage(void) {
  fprint(stderr, "usage: wc [-c|-m] [-lw] [file...]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  cflag = lflag = mflag = wflag = 0;
  ARGBEGIN("clmw"){
  case 'c':
    cflag = 1;
    break;
  case 'm':
    mflag = 1;
    break;
  case 'l':
    lflag = 1;
    break;
  case 'w':
    wflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(cflag+mflag > 1)
    usage();
  if(!(cflag+lflag+mflag+wflag))
    cflag = lflag = wflag = 1;
  if(argc == 0)
    wc(stdin, "<stdin>");
  for(i = 0; i < argc; i++) {
    f = fopen(argv[i], "r");
    if(f == nil) {
      alert("can't open %s: %m", argv[i]);
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
    if(mflag)
      printf(" %7d", totc);
    println(" total");
  }
  exit(rval);
}
