#include <u.h>
#include <avian.h>
#include <sys/stat.h>

#define B64_LEN  72
#define HIST_LEN 60

void base64dec(char *);
void histdec(char *);
int  lookup(int);
int  openpath(char *);
void pack(char[4], char[3]);

char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz0123456789+/";
char dec[3], raw[4];
char *inam, *onam;
void (*uudecode)(char *);

void
base64dec(char *s) {
  static int i;
  int n, stop;
  char *p;

  stop = 0;
  for(p = s; *p; p++) {
    if(*p == '=')
      stop = 1;
    if(stop == 0)
      raw[i++] = lookup(*p);
    if(i == 4 || stop) {
      n = 3;
      if(i == 2)
        n--;
      pack(raw, dec);
      if(fwrite(dec, 1, n, stdout) != n)
        fatal(1, "write %s: %m", onam);
      if(stop)
        return;
      i = 0;
      memset(raw, 0, 4);
    }
  }
}

void
histdec(char *s) {
  int i, j, len, m, n;

  i = 0;
  len = *s++ - 0x20;
  m = len % 3;
  len = len / 3 * 4;
  if(m != 0)
    len += m + 1; /* <- correct ? */
  for(j = 0; j < len; j++) {
    raw[i++] = s[j] - 0x20;
    if(i == 4 || j == len-1) {
      n = 3;
      if(i == 2)
        n--;
      pack(raw, dec);
      if(fwrite(dec, 1, n, stdout) != n)
        fatal(1, "write %s: %m", onam);
      if(n < 3)
        return;
      i = 0;
      memset(raw, 0, 4);
    }
  }
}

int
lookup(int c) {
  int i;

  for(i = 0; i < 64; i++)
    if(b64[i] == c)
      return i;
  return -1;
}

int
openpath(char *s) {
  uint mod;
  char *p;

  p = strchr(s, ' ');
  if(p == nil)
    return -1;
  *p++ = '\0';
  mod = symmod(0, s);
  if(mod == 0)
    return -1;
  if(onam == nil)
    onam = strdup(p);
  if(!strcmp(onam, "/dev/stdout") || !strcmp(onam, "-"))
    return 0;
  if(!freopen(onam, "w", stdout))
    fatal(1, "open %s: %m", onam);
  if(fchmod(fileno(stdout), mod))
    fatal(1, "chmod %s: %m", onam);
  return 0;
}

void
pack(char bin[4], char bout[3]) {
  bout[0] = (bin[0]<<2)|(bin[1]>>4);
  bout[1] = (bin[1]<<4)|(bin[2]>>2);
  bout[2] = (bin[2]<<6)|bin[3];
}

void
usage(void) {
  fprint(stderr, "usage: uudecode [-o outfile] [file]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *end, *p;
  int parse;

  onam = nil;
  ARGBEGIN("o:"){
  case 'o':
    onam = optarg;
    break;
  default:
    usage();
  }ARGEND

  if(argc > 1)
    usage();
  inam = "<stdin>";
  if(argc) {
    inam = argv[0];
    if(!freopen(argv[0], "r", stdin))
      fatal(1, "open %s: %m", argv[0]);
  }
  parse = 0;
  while((p = fgetln(stdin))) {
    if(parse) {
      if(!strcmp(p, end))
        break;
      uudecode(p);
      continue;
    }
    if(!strncmp(p, "begin", 5)) {
      p += 5;
      if(!strncmp(p, "-base64 ", 8)) {
        end = "====";
        p += 8;
        uudecode = base64dec;
      } else {
        end = "end";
        p++;
        uudecode = histdec;
      }
      if(openpath(p) == 0)
        parse = 1;
    }
  }
  if(ferror(stdin))
    fatal(1, "read %s: %m", inam);
  if(parse == 0)
    fatal(1, "%s is not encoded", inam);
  exit(0);
}
