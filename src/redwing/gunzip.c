#include <u.h>
#include <avian.h>
#include <zlib.h>

int cflag, vflag;

int
chkfmt(FILE *f) {
  uchar magic[2];

  if(fread(magic, 1, 2, f) != 2 || magic[0] != 0x1f || magic[1] != 0x8b)
    return -1;
  rewind(f);
  return 0;
}

void
gunzip(int ifd, char *s) {
  char buf[8192], *name, *p;
  gzFile in;
  uint n;
  FILE *out;

  in = gzdopen(ifd, "rb");
  if(in == nil) {
    alert("open %s: %m", s);
    return;
  }
  if(cflag) {
    name = "<stdout>";
    out = stdout;
  } else {
    name = strdup(s);
    p = strrchr(name, '.');
    if(p) {
      if(!strcmp(p, ".tgz"))
        strcpy(p, ".tar");
      else if(!strcmp(p, ".gz"))
        *p = '\0';
    }
    if(!strcmp(name, s)) {
      alert("can't overwrite %s", s);
      goto End;
    }
    out = fopen(name, "w");
    if(out == nil) {
      alert("open %s: %m", name);
      goto End;
    }
  }
  if(vflag)
    fprintf(stderr, "decompressing %s to %s\n", s, name);
  while((n = gzread(in, buf, sizeof buf)) > 0)
    if(fwrite(buf, 1, n, out) != n) {
      alert("write %s: %m", name);
      goto End;
    }
  if(n < 0)
    alert("read %s: %m", s);
End:
  gzclose(in);
  if(!cflag)
    free(name);
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  cflag = vflag = 0;
  ARGBEGIN("cv"){
  case 'c':
    cflag = 1;
    break;
  case 'v':
    vflag = 1;
    break;
  default:
    fprint(stderr, "usage: gunzip [-cv] [file...]\n");
    exit(1);
  }ARGEND

  if(argc == 0) {
    cflag = 1;
    gunzip(0, "<stdin>");
  }
  for(i = 0; i < argc; i++) {
    f = fopen(argv[i], "r");
    if(f == nil) {
      alert("open %s: %m", argv[i]);
      continue;
    }
    if(chkfmt(f))
      alert("%s is not a gzip deflate file", argv[i]);
    else
      gunzip(fileno(f), argv[i]);
    fclose(f);
  }
  exit(rval);
}
