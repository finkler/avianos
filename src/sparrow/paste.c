#include <u.h>
#include <avian.h>
#include <utf8.h>

rune *delim;
int ndelim;
int sflag;

void
list(char *s) {
  int i;
  char *p;

  ndelim = utflen(s);
  for(p = s; *p; p++)
    if(*p == '\\' && *(p+1) != '\\')
      ndelim--;
  p = s;
  delim = malloc(ndelim*sizeof(rune));
  for(i = 0; i < ndelim; i++) {
    if(*p == '\\') {
      switch(*++p) {
      case 'n':
        delim[i] = '\n';
        break;
      case 't':
        delim[i] = '\t';
        break;
      case '\\':
        delim[i] = '\\';
        break;
      case '0':
        delim[i] = '\0';
        break;
      default:
        fatal(1, "invalid special \\%c", *p);
      }
      p++;
    } else {
      p += runedec(&delim[i], p);
    }
  }
}

void
paste(FILE **in, int n) {
  char buf[UTF_MAX+1], *p;
  int i, j;

  i = j = 0;
  while(i != n) {
    while(i < n) {
      p = fgetln(in[i]);
      if(p)
        print(p);
      else if(sflag)
        break;
      buf[runeenc(buf, delim[j++])] = '\0';
      if(j == ndelim)
        j = 0;
      print(buf);
      if(!sflag)
        i++;
    }
    print("\n");
    if(sflag) {
      j = 0;
      i++;
    } else {
      for(i = 0; i < n; i++)
        if(!feof(in[i]))
          break;
    }
  }
}

void
usage(void) {
  fprint(stderr, "usage: paste [-s] [-d list] file...\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  FILE **f;
  int i;
  rune tab;

  tab = '\t';
  delim = &tab;
  ndelim = 1;
  sflag = 0;
  ARGBEGIN("d:s"){
  case 'd':
    list(optarg);
    break;
  case 's':
    sflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc < 1)
    usage();
  f = malloc(argc*sizeof(FILE *));
  for(i = 0; i < argc; i++) {
    if(!strcmp(argv[i], "-")) {
      f[i] = stdin;
    } else {
      f[i] = fopen(argv[i], "r");
      if(f[i] == nil) {
        alert("open %s: %m", argv[i]);
        if(!sflag)
          exit(rval);
      }
    }
  }
  paste(f, argc);
  exit(rval);
}
