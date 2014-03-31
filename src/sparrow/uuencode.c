#include <u.h>
#include <avian.h>
#include <sys/stat.h>

#define B64_LEN  72
#define HIST_LEN 60

char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz0123456789+/";
char enc[4], raw[3];
void (*uuencode)(char[4], int);

void
base64enc(char buf[4], int n) {
  static int i;

  if(n == 0)
    return;
  buf[0] = b64[(int)buf[0]];
  buf[1] = b64[(int)buf[1]];
  buf[2] = n > 1 ? b64[(int)buf[2]] : '=';
  buf[3] = n > 2 ? b64[(int)buf[3]] : '=';
  fwrite(buf, 1, 4, stdout);
  i += 4;
  if(i == B64_LEN) {
    i = 0;
    print("\n");
  }
}

void
histenc(char bin[4], int n) {
  static char bout[HIST_LEN+1];
  static int i, len;
  int m;

  if(n > 0) {
    bin[0] += 0x20;
    bin[1] += 0x20;
    bin[2] += 0x20;
    bin[3] += 0x20;
    m = 4;
    if(n < 3)
      m--;
    if(n < 2)
      m--;
    memcpy(bout+i, bin, m);
    i += m;
    len += n;
  }
  if(i == HIST_LEN || n == 0) {
    bout[i] = '\0';
    printf("%c%s", len+0x20, bout);
    if(n == 0)
      return;
    i = len = 0;
    print("\n");
  }
}

void
unpack(char bin[3], char bout[4]) {
  bout[0] = (bin[0]>>2)&0x3F;
  bout[1] = ((bin[0]<<4)|((bin[1]>>4)&0xF))&0x3F;
  bout[2] = ((bin[1]<<2)|((bin[2]>>6)&0x3))&0x3F;
  bout[3] = bin[2]&0x3F;
}

void
usage(void) {
  fprint(stderr, "usage: uuencode [-m] [file] decode_pathname\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *begin, *end, *name;
  int n;
  struct stat sb;

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
  name = "<stdin>";
  if(argc == 2) {
    name = argv[0];
    if(!freopen(argv[0], "r", stdin))
      fatal(1, "open %s: %m", argv[0]);
    argv++;
  }
  if(fstat(0, &sb))
    fatal(1, "stat %s: %m", name);
  printf("%s %o %s\n", begin, sb.st_mode&0x1FF, argv[0]);
  for(;;) {
    n = fread(raw, 1, 3, stdin);
    if(n < 0)
      fatal(1, "read %s: %m", name);
    unpack(raw, enc);
    uuencode(enc, n);
    if(n == 0)
      break;
    memset(raw, 0, 3);
  }
  printf("\n%s\n", end);
  exit(0);
}
