#include <u.h>
#include <avian.h>
#include <sys/stat.h>

void
usage(void) {
  fprint(stderr, "usage: cmp [-l|-s] file1 file2\n");
  exit(2);
}

int
main(int argc, char *argv[]) {
  int c[2], i;
  int lflag, sflag;
  FILE *f[2];
  struct stat sb[2];
  vlong byte, line;

  lflag = sflag = 0;
  ARGBEGIN("ls"){
  case 'l':
    lflag = 1;
    break;
  case 's':
    sflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc != 2)
    usage();
  for(i = 0; i < 2; i++)
    if(!strcmp(argv[i], "-")) {
      f[i] = stdin;
    } else {
      f[i] = fopen(argv[i], "r");
      if(f[i] == nil)
        fatal(2, "open %s: %m", argv[i]);
    }
  if(lflag) {
    for(i = 0; i < 2; i++)
      if(fstat(fileno(f[i]), &sb[i]))
        fatal(2, "stat %s: %m", argv[i]);
    if(sb[0].st_size != sb[1].st_size) {
      if(!sflag)
        alert("EOF on %s", sb[0].st_size<sb[1].st_size?argv[0]:argv[1]);
      exit(1);
    }
  }
  for(byte = line = 1; !feof(f[0]); byte++) {
    for(i = 0; i < 2; i++) {
      c[i] = fgetc(f[i]);
      if(c[i] == EOF && ferror(f[i]))
        fatal(2, "read %s: %m", argv[i]);
    }
    if(feof(f[0]) != feof(f[1])) {
      if(!sflag)
        alert("EOF on %s", feof(f[0])?argv[0]:argv[1]);
      exit(1);
    }
    if(c[0] != c[1]) {
      if(sflag)
        exit(1);
      if(lflag) {
        printf("%lld %o %o\n", byte, c[0], c[1]);
        rval = 1;
      } else {
        printf("%s %s differ: char %lld, line %lld\n",
          argv[0], argv[1], byte, line);
        exit(1);
      }
    }
    if(c[0] == '\n')
      line++;
  }
  exit(rval);
}
