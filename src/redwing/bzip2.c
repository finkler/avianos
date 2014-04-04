#include <u.h>
#include <avian.h>
#include <bzlib.h>
#include <sys/stat.h>

int cflag;
int vflag;
int level;

void
bzip2(FILE *in, char *s)
{
  char buf[8192], *name, *p, *suffix;
  BZFILE *b;
  int err;
  uint n;
  FILE *out;

  b = nil;
  if(cflag){
    name = "<stdout>";
    out = stdout;
  }else{
    suffix = ".bz2";
    p = strrchr(s, '.');
    if(p && !strcmp(p, ".tar")){
      *p = '\0';
      suffix = ".tbz";
    }
    name = stradd(s, suffix);
    out = fopen(name, "w");
    if(out == nil){
      alert("open %s: %m", name);
      goto End;
    }
  }
  b = BZ2_bzWriteOpen(&err, out, level, 0, 0);
  if(err != BZ_OK){
    alert("open %s: %m", name);
    goto End;
  }
  if(vflag)
    fprintf(stderr, "compressing %s to %s\n", s, name);
  while((n = fread(buf, 1, sizeof buf, in)) > 0){
    BZ2_bzWrite(&err, b, buf, n);
    if(err != BZ_OK){
      alert("write %s: %m", name);
      goto End;
    }
  }
  if(ferror(in))
    alert("read %s: %m", s);
End:
  BZ2_bzWriteClose(&err, b, 0, nil, nil);
  if(err != BZ_OK)
    alert("write %s: %m", name);
  if(!cflag){
    free(name);
    fclose(out);
  }
}

int
chkfmt(char *s)
{
  struct stat sb;

  if(stat(s, &sb)){
    alert("stat %s: %m", s);
    return -1;
  }
  if(S_ISDIR(sb.st_mode)){
    alert("can't compress a directory");
    return -1;
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  FILE *f;
  int i;

  cflag = vflag = 0;
  level = 6;
  ARGBEGIN("123456789cv"){
  case '1': case '2': case '3':
  case '4': case '5': case '6':
  case '7': case '8': case '9':
    level = c - '0';
    break;
  case 'c':
    cflag = 1;
    break;
  case 'v':
    vflag = 1;
    break;
  default:
    fprint(stderr, "usage: bzip2 [-cv[1-9]] [file...]\n");
    exit(1);
  }ARGEND

  if(argc == 0){
    cflag = 1;
    bzip2(stdin, "<stdin>");
  }
  for(i = 0; i < argc; i++){
    if(chkfmt(argv[i]))
      continue;
    f = fopen(argv[i], "r");
    if(f == nil){
      alert("open %s: %m", argv[i]);
      continue;
    }
    bzip2(f, argv[i]);
    fclose(f);
  }
  exit(rval);
}
