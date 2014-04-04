#include <u.h>
#include <avian.h>

static void vlog(char*, va_list);

int rval = EXIT_SUCCESS;

void
alert(char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vlog(fmt, ap);
  va_end(ap);
  rval = EXIT_FAILURE;
}

void
fatal(int c, char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vlog(fmt, ap);
  va_end(ap);
  exit(c);
}

void
vlog(char *fmt, va_list ap)
{
  int n;

  if(argv0)
    fprintf(stderr, "%s: ", argv0);
  vfprintf(stderr, fmt, ap);
  n = strlen(fmt);
  if(n > 0 && fmt[n-1] != '\n')
    fprint(stderr, "\n");
}
