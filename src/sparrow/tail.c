#include <u.h>
#include <avian.h>
#include <sys/stat.h>

typedef union {
  char c;
  char *s;
} Buffer;

int byte, len;
Buffer *buf;

void
follow(char *s, int isstdin) {
  int c;
  struct stat sb;

  if(fstat(0, &sb))
    fatal(1, "can't follow %s: %m", s);
  if((!S_ISREG(sb.st_mode) && !S_ISFIFO(sb.st_mode)) ||
    (S_ISFIFO(sb.st_mode) && isstdin))
    fatal(1, "can't follow %s: not supported", s);
  for(;;) {
    sleep(3);
    while((c = fgetc(stdin)) != EOF) {
      if(fputc(c, stdout) == EOF)
        fatal(1, "error writing <stdout>: %m");
    }
    if(ferror(stdin))
      fatal(1, "error reading %s: %m", s);
    clearerr(stdin);
    fflush(stdout);
  }
}

char *
readinput(void) {
  static char c, line[LINE_MAX+1];
  int n;

  n = 0;
  while(n < LINE_MAX) {
    c = fgetc(stdin);
    if(c == EOF)
      return nil;
    if(byte)
      return &c;
    line[n++] = c;
    if(c == '\n')
      break;
  }
  line[n] = '\0';
  return line;
}

void
usage(void) {
  fprint(stderr, "usage: tail [-f] [-c number|-n number] [file]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int fflag, i, head, n;
  char *p;

  for(i = 1; i < argc && strcmp(argv[i], "--"); i++)
    if(argv[i][0] == '+' && !isdigit(argv[i][1]))
      argv[i][0] = '-';
  byte = fflag = head = 0;
  len = 10;
  ARGBEGIN("c:fn:"){
  case 'c':
    byte = 1;
  case 'n':
    switch(*optarg) {
    case '+':
      head = 1;
    case '-':
      optarg++;
    }
    len = atoi(optarg);
    break;
  case 'f':
    fflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  
  if(argc > 1)
    usage();
  if(argc == 1 && !freopen(argv[0], "r", stdin))
    fatal(1, "can't open %s: %m", argv[0]);
  buf = malloc(len*sizeof(Buffer));
  for(n = 0; (p = readinput()); n++) {
    if(n == len) {
      if(head)
        break;
      if(byte == 0)
        free(buf[0].s);
      for(n = 0; n < len-1; n++)
        buf[n] = buf[n+1];
    }
    if(byte)
      buf[n].c = *p;
    else
      buf[n].s = strdup(p);
  }
  for(i = 0; i < n; i++)
    if(byte)
      printf("%c", buf[i].c);
    else
      print(buf[i].s);
  if(fflag)
    follow(argc ? argv[0] : "<stdin>", !argc);
}
