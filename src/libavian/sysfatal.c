#include <u.h>
#include <avian.h>
#include <syslog.h>

static void __vsyslog(int, char*, va_list);

void
sysalert(char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  __vsyslog(LOG_WARNING, fmt, ap);
  va_end(ap);
  rval = EXIT_FAILURE;
}

void
sysfatal(int c, char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  __vsyslog(LOG_ERR, fmt, ap);
  va_end(ap);
  exit(c);
}

void
__vsyslog(int prio, char *fmt, va_list ap)
{
  char buf[512];
  int n;

  n = vsnprintf(buf, sizeof buf, fmt, ap);
  if(n > 0 && buf[n-1] == '\n')
    buf[n-1] = '\0';
  openlog(argv0, 0, LOG_USER);
  syslog(prio, "%s\n", buf);
  closelog();
}
