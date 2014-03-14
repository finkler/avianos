#include <u.h>
#include <avian.h>
#include <bzlib.h>

int cflag, vflag;
int debug;

void
bunzip2(FILE *in, char *s) {
  char buf[8192], *name, *p;
  BZFILE *b;
  int err;
  uint n;
  FILE *out;

  b = BZ2_bzReadOpen(&err, in, 0, debug, nil, 0);
  if(err != BZ_OK) {
    alert("open %s: %m", s);
    return;
  }
  out = nil;
  if(cflag) {
    name = "<stdout>";
    out = stdout;
  } else {
    name = strdup(s);
    p = strrchr(name, '.');
    if(p) {
      if(!strcmp(p, ".tbz"))
        strcpy(p, ".tar");
      else if(!strcmp(p, ".bz2"))
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
  for(;;) {
    n = BZ2_bzRead(&err, b, buf, sizeof buf);
    if(err != BZ_OK)
      break;
    if(fwrite(buf, 1, n, out) != n) {
      alert("write %s: %m", name);
      goto End;
    }
  }
  if(err != BZ_STREAM_END)
    alert("read %s: %m", s);
End:
  BZ2_bzReadClose(&err, b);
  if(!cflag) {
    free(name);
    fclose(out);
  }
}

int
chkfmt(FILE *f) {
  uchar magic[2];

  if(fread(magic, 1, 2, f) != 2 || magic[0] != 0x42 || magic[1] != 0x5a)
    return -1;
  rewind(f);
  return 0;
}

int
main(int argc, char *argv[]) {
  FILE *f;
  int i;

  cflag = vflag = 0;
  debug = 0;
  ARGBEGIN("cvD"){
  case 'c':
    cflag = 1;
    break;
  case 'v':
    vflag = 1;
    break;
  case 'D':
    debug = 1;
    break;
  default:
    fprint(stderr, "usage: bunzip2 [-cvD] [file...]\n");
    exit(1);
  }ARGEND

  if(argc == 0) {
    cflag = 1;
    bunzip2(stdin, "<stdin>");
  }
  for(i = 0; i < argc; i++) {
    f = fopen(argv[i], "r");
    if(f == nil) {
      alert("open %s: %m", argv[i]);
      continue;
    }
    if(chkfmt(f))
      alert("%s is not a bzip2 file", argv[i]);
    else
      bunzip2(f, argv[i]);
    fclose(f);
  }
  exit(rval);
}
