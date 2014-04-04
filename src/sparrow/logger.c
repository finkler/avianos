#include <u.h>
#include <avian.h>
#include <syslog.h>

void
usage(void)
{
  fprint(stderr, "usage: logger string...\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  char *buf, *p;
  int i, len;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND

  if(argc < 1)
    usage();
  len = 1;
  for(i = 0; i < argc; i++)
    len += strlen(argv[i])+1;
  buf = malloc(len);
  p = buf;
  for(i = 0; i < argc; i++){
    if(i > 0)
      *p++ = ' ';
    strcpy(p, argv[i]);
    p += strlen(p);
  }
  openlog(getlogin(), 0, LOG_USER);
  syslog(LOG_NOTICE, "%s", buf);
  closelog();
}
