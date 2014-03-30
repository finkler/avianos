#include <u.h>
#include <avian.h>
#include <strings.h>
#include <utf8.h>

#define FLTLEN sizeof(long double)
#define INTLEN sizeof(vlong)
#define INCR   10
#define NUM    16

typedef struct Dfmt Dfmt;
struct Dfmt {
  void (*dump)(Dfmt *, char *, int);
  int field;
  int size;
  int type;
};

typedef char Float[FLTLEN];

typedef union Int {
  vlong val;
  char byte[INTLEN];
} Int;

char addr[6];
char *cname[] = {
  "nul", "soh", "stx", "etx", "eot", "enq", "ack",
  "bel", "bs", "ht", "nl", "vt", "ff", "cr", "so",
  "si", "dle", "dc1", "dc2", "dc3", "dc4", "nak",
  "syn", "etb", "can", "em", "sub", "esc", "fs",
  "gs", "rs", "us", "sp", "del"
};
uint count, minsize, skip, width;
Dfmt *dfmt;
int fields[8][4] = {
  /* x */ { 2,  4,  8, 16},
  /* a */ { 3,  3,  3,  3},
  /* - */ { 0,  0,  0,  0},
  /* c */ { 3,  3,  3,  3},
  /* d */ { 4,  6, 11, 20},
  /* u */ { 3,  5, 10, 20},
  /* f */ {28,  0, 14, 21},
  /* o */ { 3,  6, 11, 22}
};
int len;
int nflag, vflag;

void
dumpascii(Dfmt *d, char *buf, int n) {
  int c, k;

  for(k = 0; k < n; k++) {
    c = buf[k]&0x7F;
    if(isprint(c)) {
      printf("   %c", c);
    } else {
      if(c == 0x7F)
        c = 0x21;
      printf(" %3s", cname[c]);
    }
  }
  print("\n");
}

void
dumpchar(Dfmt *d, char *buf, int n) {
  char utf[UTF_MAX+1];
  int k, nr;

  k = 0;
  while(k < n) {
    nr = fullrune(buf[k]);
    if(nr == 0 || k+nr > n) {
      printf(" %03o", buf[k]);
      k++;
      continue;
    }
    strncpy(utf, buf+k, nr);
    utf[nr] = '\0';
    k += nr;
    switch(*utf) {
    case '\0':
      print("  \\0");
      break;
    case '\\':
      print("  \\\\");
      break;
    case '\a':
      print("  \\a");
      break;
    case '\b':
      print("  \\b");
      break;
    case '\f':
      print("  \\f");
      break;
    case '\n':
      print("  \\n");
      break;
    case '\r':
      print("  \\r");
      break;
    case '\t':
      print("  \\t");
      break;
    case '\v':
      print("  \\v");
      break;
    default:
      if(!isprint(*utf)) {
        printf(" %03o", (uchar)*utf);
      } else {
        printf("   %s", utf);
        while(--nr > 0)
          print("  **");
      }
    }
  }
  print("\n");
}

void
dumpfloat(Dfmt *d, char *buf, int n) {
  char num[64];
  Float f;
  int j, k, field;

  field = d->size / minsize * width;
  if(len > 1)
    field += d->size - 1;
  j = 0;
  while(j < n) {
    memset(f, 0, FLTLEN);
    for(k = 0; k < d->size && j < n; k++)
      f[k] = buf[j++];
    switch(d->size) {
    case 4:
      sprintf(num, "%14.7e", *(float *)&f);
      break;
    case 8:
      sprintf(num, "%21.14le", *(double *)&f);
      break;
    default:
      sprintf(num, "%28.21Le", *(long double *)&f);
      break;
    }
    printf(" %*s", field, num);
  }
  print("\n");
}

void
dumpint(Dfmt *d, char *buf, int n) {
  char fmt[16], num[64];
  Int i;
  int j, k, field;

  field = d->size / minsize * width;
  if(len > 1)
    field += d->size - 1;
  if(d->type == 'x' || d->type == 'o')
    sprintf(fmt, "%%0%dll%c", d->field, d->type);
  else
    sprintf(fmt, "%%ll%c", d->type);
  j = 0;
  while(j < n) {
    i.val = 0;
    for(k = 0; k < d->size && j < n; k++)
      i.byte[k] = buf[j++];
    sprintf(num, fmt, i.val);
    printf(" %*s", field, num);
  }
  print("\n");
}

int
freadblk(char *buf, FILE *in) {
  static uint cnt;
  int i;

  if(count) {
    for(i = 0; i < NUM && cnt < count; i++, cnt++) {
      buf[i] = fgetc(in);
      if(buf[i] == EOF)
        break;
    }
    return i;
  }
  return fread(buf, 1, NUM, in);
}

void
od(FILE *in, char *s) {
  char buf[NUM];
  int i, n, off;

  while(skip) {
    n = fread(buf, 1, 1, in);
    if(n < 0) {
      alert("read %s: %m", s);
      return;
    }
    skip--;
    if(n == 0)
      return;
  }
  off = 0;
  while((n = freadblk(buf, in)) > 0) {
    if(!nflag)
      printf(addr, off);
    for(i = 0; i < len; i++) {
      if(i > 0 && !nflag)
        print("       ");
      dfmt[i].dump(&dfmt[i], buf, n);
    }
    off += n;
  }
  if(!nflag) {
    printf(addr, off);
    print("\n");
  }
  if(ferror(in))
    alert("read %s: %m", s);
}

void
tappend(char *s) {
  static int cap;
  int mod, n, siz;

  if(cap == 0) {
    cap = INCR;
    dfmt = malloc(cap*sizeof(Dfmt));
  }
  if(cap == len) {
    cap += INCR;
    dfmt = realloc(dfmt, cap*sizeof(Dfmt));
  }
  mod = *s;
  siz = *(s+1);
  switch(mod) {
  case 'a':
    dfmt[len].dump = dumpascii;
    if(siz)
      fatal(1, "invalid type %s", s);
    siz = 1;
    break;
  case 'c':
    dfmt[len].dump = dumpchar;
    if(siz)
      fatal(1, "invalid type %s", s);
    siz = 1;
    break;
  case 'f':
    dfmt[len].dump = dumpfloat;
    switch(siz) {
    case '\0':
      siz = sizeof(double);
      break;
    case '4': case '8':
      siz -= '0';
      break;
    case 'F':
      siz = sizeof(float);
      break;
    case 'D':
      siz = sizeof(double);
      break;
    case 'L':
      siz = sizeof(long double);
      break;
    default:
      siz = strtol(s+1, nil, 10);
      if(siz == FLTLEN)
        break;
      fatal(1, "invalid type %s", s);
    }
    break;
  case 'd': case 'o':
  case 'u': case 'x':
    dfmt[len].dump = dumpint;
    switch(siz) {
    case '\0':
      siz = sizeof(int);
      break;
    case '1': case '2':
    case '4': case '8':
      siz -= '0';
      break;
    case 'C':
      siz = sizeof(char);
      break;
    case 'S':
      siz = sizeof(short);
      break;
    case 'I':
      siz = sizeof(int);
      break;
    case 'L':
      siz = sizeof(long);
      break;
    default:
      fatal(1, "invalid type %s", s);
    }
    break;
  default:
    fatal(1, "invalid type %s", s);
  }
  if(minsize > siz)
    minsize = siz;
  n = siz == FLTLEN ? 0 : ffs(siz) - 1;
  n = fields[mod%8][n];
  if(siz == minsize && width > n)
    width = n;
  dfmt[len].field = n;
  dfmt[len].type = mod;
  dfmt[len].size = siz;
  len++;
}

int
main(int argc, char *argv[]) {
  char *e;
  FILE *f;
  int i;

  strcpy(addr, "%07d");
  count = skip = 0;
  len = 0;
  minsize = width = (uint)-1;
  nflag = vflag = 0;
  ARGBEGIN("A:j:N:t:v"){
  case 'A':
    if(optarg[1] != '\0')
      fatal(1, "invalid address base");
    switch(optarg[0]) {
    case 'd': case 'o':
      sprintf(addr, "%%07%c", optarg[0]);
      break;
    case 'x':
      strcpy(addr, "%06x ");
      break;
    case 'n':
      nflag = 1;
      break;
    default:
      fatal(1, "invalid address base");
    }
    break;
  case 'j':
    skip = strtoul(optarg, &e, 0);
    switch(*e) {
    case '\0':
      break;
    case 'b':
      skip *= 512;
      break;
    case 'k':
      skip *= 1024;
      break;
    case 'm':
      skip *= 1048576;
      break;
    default:
      fatal(1, "invalid jump");
    }
    break;
  case 'N':
    count = strtoul(optarg, nil, 0);
    break;
  case 't':
    tappend(optarg);
    break;
  case 'v':
    vflag = 1;
    break;
  default:
    fprint(stderr, "usage: od [-v] [-A address_base] [-j skip] "
      "[-N count] [-t type_string]...\n"
      "  [file...]\n");
    exit(1);
  }ARGEND

  if(len == 0)
    tappend("oS");
  if(argc == 0)
    od(stdin, "<stdin>");
  for(i = 0; i < argc; i++)
    if(!strcmp(argv[i], "-")) {
      od(stdin, "<stdin>");
    } else {
      f = fopen(argv[i], "r");
      if(f == nil) {
        alert("open %s: %m", argv[i]);
        continue;
      }
      od(f, argv[i]);
      fclose(f);
    }
  if(skip)
    fatal(1, "jump is out of range");
  exit(rval);
}
