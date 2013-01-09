#include <u.h>
#include <avian.h>

void
printw(char *fmt, ...) {
  static int len, tw;
  va_list ap;
  char buf[256];
  int n;

  if(fmt == nil) {
    print("\n");
    len = 0;
    return;
  }
  if(tw == 0)
    tw = textwidth();
  va_start(ap, fmt);
  n = vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);
  if(len && len+n >= tw) {
    print("\n");
    len = 0;
  }
  if(len)
    print(" ");
  print(buf);
  len += n;
}
