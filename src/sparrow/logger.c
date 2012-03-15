#include <avian.h>
#include <syslog.h>

void
usage(void) {
  fprint("usage: logger string...\n", stderr);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *buf;
  int i, n;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND 
  if(argc < 1)
    usage();
  for(i = n = 0; i < argc; i++)
    n += strlen(argv[i]) + 1;
  buf = malloc(n);
  for(i = 0; i < argc; i++)
    if(i == 0) {
      strcpy(buf, argv[i]);
    } else {
      strcat(buf, " ");
      strcat(buf, argv[i]);
    }
  openlog(getlogin(), 0, LOG_USER);
  syslog(LOG_NOTICE, "%s", buf);
  closelog();
}
