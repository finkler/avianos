#include <avian.h>
#include <termios.h>

char *
readcons(char *p, char *def, int secret) {
  char *buf;
  struct termios tc;

  if(!isatty(0) || !isatty(2))
    return nil;
  if(def && *def)
    fprintf(stderr, "%s[%s]: ", p, def);
  else
    fprintf(stderr, "%s: ", p);
  fflush(stderr);
  if(secret) {
    tcgetattr(0, &tc);
    tc.c_lflag &= ~ECHO;
    tcsetattr(0, TCSAFLUSH, &tc);
  }
  buf = fgetln(stdin);
  if(secret) {
    tc.c_lflag |= ECHO;
    tcsetattr(0, TCSAFLUSH, &tc);
    fprint(stderr, "\n");
  }
  return buf;
}
