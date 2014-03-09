#include <u.h>
#include <avian.h>
#include <sys/stat.h>

int
binary(char *op, char *s1, char *s2) {
  char *e1, *e2;
  long n1, n2;

  if(!strcmp(op, "="))
    return strcmp(s1, s2);
  if(!strcmp(op, "!="))
    return !strcmp(s1, s2);
  n1 = strtol(s1, &e1, 0);
  n2 = strtol(s2, &e2, 0);
  if(*e1 != '\0' || *e2 != '\0')
    fatal(2, "%s: not a number", *e1?s1:s2);
  if(!strcmp(op, "-eq"))
    return n1 != n2;
  if(!strcmp(op, "-ne"))
    return n1 == n2;
  if(!strcmp(op, "-gt"))
    return n1 <= n2;
  if(!strcmp(op, "-ge"))
    return n1 < n2;
  if(!strcmp(op, "-lt"))
    return n1 >= n2;
  if(!strcmp(op, "-le"))
    return n1 > n2;
  fatal(2, "invalid operator %s", op);
}

int
unary(char *op, char *s) {
  int fd;
  struct stat sb;

  if(!strcmp(op, "-b"))
    return stat(s, &sb) || !S_ISBLK(sb.st_mode);
  if(!strcmp(op, "-c"))
    return stat(s, &sb) || !S_ISCHR(sb.st_mode);
  if(!strcmp(op, "-d"))
    return stat(s, &sb) || !S_ISDIR(sb.st_mode);
  if(!strcmp(op, "-e"))
    return access(s, F_OK);
  if(!strcmp(op, "-f"))
    return stat(s, &sb) || !S_ISREG(sb.st_mode);
  if(!strcmp(op, "-g"))
    return stat(s, &sb) || !(sb.st_mode & S_ISGID);
  if(!strcmp(op, "-h") || !strcmp(op, "-L"))
    return lstat(s, &sb) || !S_ISLNK(sb.st_mode);
  if(!strcmp(op, "-n"))
    return strlen(s) == 0;
  if(!strcmp(op, "-p"))
    return stat(s, &sb) || !S_ISFIFO(sb.st_mode);
  if(!strcmp(op, "-r"))
    return access(s, R_OK);
  if(!strcmp(op, "-S"))
    return stat(s, &sb) || !S_ISSOCK(sb.st_mode);
  if(!strcmp(op, "-s"))
    return stat(s, &sb) || sb.st_size == 0;
  if(!strcmp(op, "-t"))
    return (fd = open(s, O_RDONLY)) < 0 || !isatty(fd);
  if(!strcmp(op, "-u"))
    return stat(s, &sb) || !(sb.st_mode & S_ISUID);
  if(!strcmp(op, "-w"))
    return access(s, W_OK);
  if(!strcmp(op, "-x"))
    return access(s, X_OK);
  if(!strcmp(op, "-z"))
    return strlen(s) > 0;
  fatal(2, "%s: invalid operator", op);
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
  switch(argc) {
  case 3:
    rval = binary(argv[1], argv[0], argv[2]);
    break;
  case 2:
    rval = unary(argv[0], argv[1]);
    break;
  case 1:
    rval = argv[0][0] == '\0';
    break;
  case 0:
    rval = 1;
    break;
  default:
    fatal(2, "invalid argument list");
  }
  exit((rval&0x1)^not);
}
