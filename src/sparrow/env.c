#include <u.h>
#include <avian.h>
#include <errno.h>

extern char **environ;

int
main(int argc, char *argv[]) {
  char **e, *p;

  ARGBEGIN("i"){
  case 'i':
    environ = calloc(1, sizeof(char *));
    break;
  default:
    fprint(stderr,
      "usage: env [-i] [name=value]... [utility [argument...]]\n");
    exit(1);
  }ARGEND

  for(; *argv; argv++) {
    p = strchr(*argv, '=');
    if(p == nil)
      break;
    *p++ = '\0';
    if(setenv(*argv, p, 1))
      fatal(1, "setenv %s: %m", *argv);
  }
  if(*argv) {
    execvp(*argv, argv);
    fatal(errno==ENOENT?127:126, "exec %s: %m", *argv);
  }
  for(e = environ; *e; e++)
    println(*e);
}
