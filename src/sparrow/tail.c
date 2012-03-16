#include <avian.h>
#include <sys/stat.h>

int byte, n;
char *chr, **line;

void
follow(char *s) {
  int c;
  struct stat sb;

  if(fstat(0, &sb) || (!S_ISREG(sb.st_mode) && !S_ISFIFO(sb.st_mode)))
    fatal(1, "can't follow %s", s);
  for(;;) {
    sleep(1);
    while((c = fgetc(stdin)) != EOF)
      if(fputc(c, stdout) == EOF)
        fatal(1, "error writing <stdout>: %m");
    if(ferror(stdin))
      fatal(1, "error reading %s: %m", s);
  }
}

char *
getarr(int i) {
  return(byte ? &chr[i] : line[i]);
}

void
initarr(void) {
  if(byte)
    chr = malloc(n);
  else
    line = malloc(n*sizeof(char *));
}

void
printarr(void) {
  int i;

  if(byte)
    println(chr);
  else
    for(i = 0; i < n; i++)
      println(line[i]);
}

char *
readinput(void) {
  static char buf[LINE_MAX];
  char c;
  int i;

  for(i = 0; i < LINE_MAX-1; i++) {
    if(fread(&c, 1, 1, stdin) < 1)
      return nil;
    buf[i] = c;
    if(byte || c == '\n')
      break;
  }
  buf[++i] = '\0';
  return buf;
}

void
setarr(int i, char *p) {
  if(byte)
    chr[i] = *p;
  else
    line[i] = strdup(p);
}

void
usage(void) {
	fprint("usage: tail [-f] [-c number|-n number] [file]\n", stderr);
	exit(1);
}

int
main(int argc, char *argv[]) {
  int i, head, j;
  int fflag;
  char *p;

	for(i = 1; i < argc && strcmp(argv[i], "--"); i++)
		if(argv[i][0] == '+' && !isdigit(argv[i][1]))
			argv[i][0] = '-';
	byte = fflag = head = 0;
  n = 10;
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
    n = atoi(optarg);
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
  initarr();
  i = 0;
  while((p = readinput())) {
    if(i == n) {
      if(head)
        break;
      if(!byte)
        free(getarr(0));
      for(j = 0; j < n-1; j++)
        setarr(j, getarr(j+1));
      setarr(j, p);
    } else {
      setarr(i++, p);
    }
  }
  printarr();
  if(fflag)
    follow(argc ? argv[0] : "<stdin>");
}
