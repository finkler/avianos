#include <u.h>
#include <avian.h>
#include <sys/stat.h>
#include <zlib.h>

int cflag, vflag;
int level;

int
chkfmt(char *s) {
  struct stat sb;

  if(stat(s, &sb)) {
    alert("stat %s: %m", s);
    return -1;
  }
  if(S_ISDIR(sb.st_mode)) {
    alert("can't compress a directory");
    return -1;
  }
  return 0;
}

void
gzip(FILE *in, char *s) {
  char buf[8192], *name, *p, *suffix;
  uint n;
  gzFile out;

  if(cflag) {
    name = "<stdout>";
    out = gzdopen(1, "wb");
  } else {
    suffix = ".gz";
    p = strrchr(s, '.');
    if(p && !strcmp(p, ".tar")) {
      *p = '\0';
      suffix = ".tgz";
    }
    name = stradd(s, suffix);
    out = gzopen(name, "wb");
  }
  if(out == nil) {
    alert("open %s: %m", name);
    goto End;
  }
  gzsetparams(out, level, Z_DEFAULT_STRATEGY);
  if(vflag)
    fprintf(stderr, "compressing %s to %s\n", s, name);
  while((n = fread(buf, 1, sizeof buf, in)) > 0)
    if(gzwrite(out, buf, n) != n) {
      alert("write %s: %m", name);
      goto End;
    }
  if(ferror(in))
    alert("read %s: %m", s);
End:
  gzclose(out);
  if(!cflag)
    free(name);
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  cflag = vflag = 0;
  level = 6;
  ARGBEGIN("123456789cv"){
  case '1': case '2': case '3': case '4': case '5':
  case '6': case '7': case '8': case '9':
    level = c - '0';
    break;
  case 'c':
    cflag = 1;
    break;
  case 'v':
    vflag = 1;
    break;
  default:
    fprint(stderr, "usage: gzip [-cv[1-9]] [file...]\n");
    exit(1);
  }ARGEND

  if(argc == 0) {
    cflag = 1;
    gzip(stdin, "<stdin>");
  }
  for(i = 0; i < argc; i++) {
    if(chkfmt(argv[i]))
      continue;
    f = fopen(argv[i], "r");
    if(f == nil) {
      alert("open %s: %m", argv[i]);
      continue;
    }
    gzip(f, argv[i]);
    fclose(f);
  }
  exit(rval);
}
