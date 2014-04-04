#include <u.h>
#include <avian.h>
#include <sys/stat.h>
#include <utf8.h>

int Pflag;
int pflag;
int namlen;
int pathlen;

void
pathchk(char *s)
{
  static char chset[] = "ABZDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz0123456789._-";
  rune c;
  uint n;
  char *p, *q, *r;
  struct stat sb;

  if(strlen(s) > pathlen){
    alert("%s: path too long", s);
    return;
  }
  if(lstat(s, &sb) && errno != ENOENT){
    alert("%s: not searchable");
    return;
  }
  q = strdup(s);
  if(*q == '/')
    q++;
  for(p = strtok(q, "/"); p; p = strtok(nil, "/")){
    if(Pflag && *p == '-'){
      alert("%s: name starts with -", s);
      goto End;
    }
    if(strlen(p) > namlen){
      alert("%s: name too long", s);
      goto End;
    }
    r = p;
    while(*r){
      if(pflag){
        if(strchr(chset, *r) == nil){
          alert("%s: invalid character", s);
          goto End;
        }
        r++;
      }else{
        n = runeenc(c, r);
        if(!isprint(c) || c == RUNE_ERROR){
          alert("%s: invalid byte sequence");
          goto End;
        }
        r += n;
      }
    }
  }
End:
  free(q);
}

void
usage(void)
{
  fprint(stderr, "usage: pathchk [-p] [-P] pathname...\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int i;

  Pflag = pflag = 0;
  namlen = NAME_MAX;
  pathlen = PATH_MAX;
  ARGBEGIN("Pp"){
  case 'P':
    Pflag = 1;
    break;
  case 'p':
    namelen = _POSIX_NAME_MAX;
    pathlen = _POSIX_PATH_MAX;
    pflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc < 1)
    usage();
  for(i = 0; i < argc; i++){
    if(Pflag && argv[i][0] == '\0')
      alert("is empty");
    else
      pathchk(cleanname(argv[i]));
  }
  exit(rval);
}
