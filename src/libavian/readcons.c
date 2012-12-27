#include <u.h>
#include <avian.h>
#include <termios.h>

static int echo(int, int);

int
echo(int fd, int toggle) {
  struct termios tc;
  int was;
  
  if(tcgetattr(fd, &tc) < 0)
    return -1;
  was = tc.c_lflag&ECHO;
  tc.c_lflag &= ~ECHO;
  tc.c_lflag |= toggle;
  if(tcsetattr(0, TCSANOW, &tc) < 0)
    return -1;
  return was;
}

char *
readcons(char *prompt, char *def, int secret) {
  char *line;
  int set;
  FILE *tty;

  tty = fopen("/dev/tty", "r+");
  if(tty == nil)
    return nil;
  if(def != nil)
    fprintf(tty, "%s[%s]: ", prompt, def);
  else
    fprintf(tty, "%s: ", prompt);
  fflush(tty);
  if(secret) {
    set = echo(fileno(tty), 0);
    if(set == -1)
      return nil;
  }
  line = fgetln(tty);
  if(secret) {
    echo(fileno(tty), set);
    fprint(tty, "\n");
  }
  fclose(tty);
  return line;
}
