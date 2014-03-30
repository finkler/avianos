#include <u.h>
#include <avian.h>
#include <errno.h>
#include <iconv.h>

#define ILEN 1024
#define OLEN 4096

iconv_t cd;
int cflag, sflag;
char *charmaps[] = {
  /* musl-libc */
  "ascii",
  "big5",
  "cp1250",
  "cp1251",
  "cp1252",
  "cp1253",
  "cp1254",
  "cp1255",
  "cp1256",
  "cp1257",
  "cp1258",
  "eucjp",
  "euckr",
  "gb18030",
  "gb2312",
  "gbk",
  "iso88591",
  "iso885910",
  "iso885911",
  "iso885913",
  "iso885914",
  "iso885915",
  "iso885916",
  "iso88592",
  "iso88593",
  "iso88594",
  "iso88595",
  "iso88596",
  "iso88597",
  "iso88598",
  "iso88599",
  "koi8r",
  "koi8u",
  "latin1",
  "latin9",
  "shiftjis",
  "tis620",
  "ucs2",
  "ucs2le",
  "ucs4",
  "ucs4le",
  "utf16",
  "utf16le",
  "utf8",
  "wchart",
  "windows1250",
  "windows1251",
  "windows1252",
  "windows1253",
  "windows1254",
  "windows1255",
  "windows1256",
  "windows1257",
  "windows1258",
  nil
};

void
convert(FILE *in, char *s) {
  char bin[ILEN], bout[OLEN], *p, *q;
  uint n;
  int nr, nw;

  while((nr = fread(bin, 1, ILEN, in)) > 0) {
    nw = OLEN;
    p = bin;
    q = bout;
    for(;;) {
      n = iconv(cd, &p, (uint *)&nr, &q, (uint *)&nw);
      if(nr == 0) {
        n = OLEN - nw;
        if(fwrite(bout, 1, n, stdout) != n) {
          alert("write <stdout>: %m");
          return;
        }
        break;
      }
      if(n == (uint)-1)
        switch(errno) {
        case EILSEQ:
          if(cflag) {
            p++; nr--;
            continue;
          }
          if(sflag)
            rval = 1;
          else
            alert("invalid character in %s", s);
          return;
        case E2BIG:
          n = OLEN - nw;
          if(fwrite(bout, 1, n, stdout) != n) {
            alert("write <stdout>: %m");
            return;
          }
          nw = OLEN;
          q = bout;
          continue;
        default:
          alert("%m");
          return;
        }
    }
  }
  if(ferror(in))
    alert("read %s: %m", s);
}

void
list(void) {
  uint n;
  char **p, *term;
  int w;

  n = 0;
  w = textwidth();
  for(p = charmaps; *p; p++) {
    n += strlen(*p) + 2;
    term = nil;
    if(*(p+1) == nil)
      term = "\n";
    if(n > w) {
      if(term == nil)
        term = ",\n";
      n = 0;
    } else if(term == nil)
      term = ", ";
    printf("%s%s", *p, term);
  }
  exit(0);
}

void
usage(void) {
  fprint(stderr, "usage: iconv [-cs] -f frommap -t tomap [file...]\n"
    "       iconv -f fromcode [-cs] [-t tocode] [file...]\n"
    "       iconv -t tocode [-cs] [-f fromcode] [file...]\n"
    "       iconv -l\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *from, *to;
  FILE *f;
  int i;
  int lflag;

  cflag = lflag = sflag = 0;
  from = to = nil;
  ARGBEGIN("cf:lst:"){
  case 'c':
    cflag = 1;
    break;
  case 'f':
    from = optarg;
    break;
  case 'l':
    lflag = 1;
    break;
  case 's':
    sflag = 1;
    break;
  case 't':
    to = optarg;
    break;
  default:
    usage();
  }ARGEND

  if(lflag) {
    if(argc != 0)
      usage();
    list();
    /* not reached */
  }
  cd = iconv_open(to, from);
  if(cd == (iconv_t)-1)
    fatal(1, "iconv_open: %m");
  if(argc == 0)
    convert(stdin, "<stdin>");
  for(i = 0; i < argc; i++)
    if(!strcmp(argv[i], "-")) {
      convert(stdin, "<stdin>");
    } else {
      f = fopen(argv[i], "r");
      if(f == nil) {
        alert("open %s: %m", argv[i]);
        continue;
      }
      convert(f, argv[i]);
      fclose(f);
    }
  iconv_close(cd);
  exit(rval);
}
