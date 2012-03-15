#include <avian.h>

char *
suffix(int n, int len) {
  static char buf[NAME_MAX+1];

  for(buf[len] = '\0'; len; n /= 26)
    buf[--len] = n % 26 + 'a';
  if(n > 0)
    fatal(1, "suffix too short");
  return buf;
}

void
usage(void) {
  fprint("usage: split [-l line_count] [-a suffix_length] [file[suffix]]\n"
        "       split -b n[k|m] [-a suffix_length] [file[suffix]]\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char buf[NAME_MAX+1], *e, *stem;
  long byte, line;
  int c, i, n, slen;
  FILE *f;

  byte = 0;
  n = line = 1000;
  slen = 2;
  stem = "x";
  ARGBEGIN("a:b:l:"){
  case 'a':
    slen = atoi(optarg);
    break;
  case 'b':
    n = byte = strtol(optarg, &e, 10);
    if(*e == 'k')
      byte <<= 10;
    else if(*e == 'm')
      byte <<= 20;
    line = 0;
    break;
  case 'l':
    byte = 0;
    n = line = atoi(optarg);
    break;
  default:
    usage();
  }ARGEND 
  if(argc > 2)
    usage();
  if(argc > 1)
    stem = argv[1];
  if(argc > 0 && strcmp(argv[0], "-"))
    if(!freopen(argv[0], "r", stdin))
      fatal(1, "can't open %s: %m", argv[0]);
  if(strlen(stem) + slen > NAME_MAX)
    fatal(1, "suffix too long");
  for(i = 0; !feof(stdin); i++) {
    sprintf(buf, "%s%s", stem, suffix(i, slen));
    if(!(f = fopen(buf, "w+")))
      fatal(1, "can't open %s: %m", buf);
    while((c = fgetc(stdin)) != EOF) {
      if(fputc(c, f) != c)
        fatal(1, "error writing %s: %m", buf);
      if(((line && c == '\n') || byte) && --n == 0)
        break;
    }
    if(ferror(stdin))
      fatal(1, "error reading %s: %m", argc > 0 ? argv[0] : "<stdin>");
    fclose(f);
  }
}
