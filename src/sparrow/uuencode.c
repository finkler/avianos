#include <u.h>
#include <avian.h>
#include <sys/stat.h>

#define B64_LEN  72
#define HIST_LEN 60

void
base64enc(char *s) {
  char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz0123456789+/";
  char enc[B64_LEN+1], buf[3];
  int i, n;

  i = 0;
  while((n = fread(buf, 1, 3, stdin)) > 0) {
    enc[i++] = b64[(buf[0]>>2)&0x3F];
    enc[i++] = b64[((buf[0]<<4)|((buf[1]>>4)&0xF))&0x3F];
    enc[i++] = n > 1 ? b64[((buf[1]<<2)|((buf[2]>>6)&0x3))&0x3F] : '=';
    enc[i++] = n > 2 ? b64[buf[2]&0x3F] : '=';
    if(i == B64_LEN) {
      enc[i] = '\0';
      printf("%s\n", enc);
      i = 0;
    }
  }
  if(n < 0)
    fatal(1, "read %s: %m", s);
  enc[i] = '\0';
  printf("%s\n", enc);
}

void
histenc(char *s) {
  char enc[HIST_LEN+1], buf[3];
  int i, n, nr;

  i = nr = 0;
  while((n = fread(buf, 1, 3, stdin)) > 0) {
    nr += n;
    enc[i++] = 0x20 + ((buf[0]>>2)&0x3F);
    enc[i++] = 0x20 + (((buf[0]<<4)|((buf[1]>>4)&0xF))&0x3F);
    if(n > 1)
      enc[i++] = 0x20 + (((buf[1]<<2)|((buf[2]>>6)&0x3))&0x3F);
    if(n > 2)
      enc[i++] = 0x20 + (buf[2]&0x3F);
    if(i == HIST_LEN) {
      enc[i] = '\0';
      printf("%c%s\n", 0x20+nr, enc);
      i = nr = 0;
    }
  }
  if(n < 0)
    fatal(1, "read %s: %m", s);
  enc[i] = '\0';
  printf("%c%s\n", 0x20+nr, enc);
}

void
usage(void) {
  fprint(stderr, "usage: uuencode [-m] [file] decode_pathname\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *begin, *end, *iname;
  struct stat sb;
  void (*uuencode)(char *);

  begin = "begin";
  end = "end";
  uuencode = histenc;
  ARGBEGIN("m"){
  case 'm':
    begin = "begin-base64";
    end = "====";
    uuencode = base64enc;
    break;
  default:
    usage();
  }ARGEND

  if(argc < 1 || argc > 2)
    usage();
  iname = "<stdin>";
  if(argc == 2) {
    iname = argv[0];
    if(!freopen(argv[0], "r", stdin))
      fatal(1, "open %s: %m", argv[0]);
    argv++;
  }
  if(fstat(0, &sb))
    fatal(1, "stat %s: %m", iname);
  printf("%s %o %s\n", begin, (sb.st_mode&0x1FF), argv[0]);
  uuencode(iname);
  printf("%s\n", end);
}
