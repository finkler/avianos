#include <avian.h>
#include <fcntl.h>
#include <sys/stat.h>

int rval;

void
binaryp(char *s1, char *op, char *s2) {
  int i1, i2;

  if(!strcmp(op, "=")) {
    rval = strcmp(s1, s2);
  } else if(!strcmp(op, "!=")) {
    rval = !strcmp(s1, s2);
  } else {
    i1 = atoi(s1);
    i2 = atoi(s2);
    if(!strcmp(op, "-eq")) {
      rval = i1 != i2;
    } else if(!strcmp(op, "-ne")) {
      rval = i1 == i2;
    } else if(!strcmp(op, "-gt")) {
      rval = i1 <= i2;
    } else if(!strcmp(op, "-ge")) {
      rval = i1 < i2;
    } else if(!strcmp(op, "-lt")) {
      rval = i1 >= i2;
    } else if(!strcmp(op, "-le")) {
      rval = i1 > i2;
    } else fatal(2, "invalid operator %s", op);
  }
}

void
unaryp(char *op, char *s) {
  int fd;
  struct stat sb;

  if(!strcmp(op, "-b")) {
    rval = stat(s, &sb) || !S_ISBLK(sb.st_mode);
  } else if(!strcmp(op, "-c")) {
    rval = stat(s, &sb) || !S_ISCHR(sb.st_mode);
  } else if(!strcmp(op, "-d")) {
    rval = stat(s, &sb) || !S_ISDIR(sb.st_mode);
  } else if(!strcmp(op, "-e")) {
    rval = access(s, F_OK);
  } else if(!strcmp(op, "-f")) {
    rval = stat(s, &sb) || !S_ISREG(sb.st_mode);
  } else if(!strcmp(op, "-g")) {
    rval = stat(s, &sb) || !(sb.st_mode & S_ISGID);
  } else if(!strcmp(op, "-h") || !strcmp(op, "-L")) {
    rval = lstat(s, &sb) || !S_ISLNK(sb.st_mode);
  } else if(!strcmp(op, "-n")) {
    rval = strlen(s) == 0;
  } else if(!strcmp(op, "-p")) {
    rval = stat(s, &sb) || !S_ISFIFO(sb.st_mode);
  } else if(!strcmp(op, "-r")) {
    rval = access(s, R_OK);
  } else if(!strcmp(op, "-S")) {
    rval = stat(s, &sb) || !S_ISSOCK(sb.st_mode);
  } else if(!strcmp(op, "-s")) {
    rval = stat(s, &sb) || sb.st_size == 0;
  } else if(!strcmp(op, "-t")) {
    rval = (fd = open(s, O_RDONLY, 0)) < 0 || !isatty(fd);
  } else if(!strcmp(op, "-u")) {
    rval = stat(s, &sb)
      !(sb.st_mode & S_ISUID);
  } else if(!strcmp(op, "-w")) {
    rval = access(s, W_OK);
  } else if(!strcmp(op, "-x")) {
    rval = access(s, X_OK);
  } else if(!strcmp(op, "-z")) {
    rval = stat(s, &sb) || sb.st_size > 0;
  } else fatal(2, "invalid operator %s", op);
}

int
main(int argc, char *argv[]) {
  int not;

  __fixargv0(argv[0]);
  argc--, argv++;
  if(!strcmp(argv0, "[") && strcmp(argv[--argc], "]"))
    fatal(2, "missing ]");

  not = 0;
  if(argc > 1 && !strcmp(argv[0], "!")) {
    not = 1;
    argc--, argv++;
  }
  rval = 1;
  if(argc == 3) {
    binaryp(argv[0], argv[1], argv[2]);
  } else if(argc ==2) {
    unaryp(argv[0], argv[1]);
  } else if(argc == 1) {
    rval = argv[0][0] == '\0';
  } else if(argc == 0) {
    rval = 1;
  } else fatal(2, "argument list too long");
  rval = rval ? 1 : 0;
  exit(rval^not);
}
