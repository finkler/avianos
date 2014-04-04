#include <u.h>
#include <avian.h>
#include <sys/ioctl.h>

#define TEXTWIDTH 72

int
textwidth(void)
{
  char *p;
  struct winsize w;

  p = getenv("COLUMNS");
  if(p)
    return atoi(p);
  if(ioctl(1, TIOCGWINSZ, &w))
    return TEXTWIDTH;
  return min(w.ws_col, TEXTWIDTH);
}
