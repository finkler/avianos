#include <u.h>
#include <avian.h>
#include <errno.h>
#include <sys/wait.h>

#define INCR 100

int pflag, tflag;

int
getarg(char *buf, int len) {
  int c, caret, n, quote;

  c = fgetc(stdin);
  while(isspace(c))
    c = fgetc(stdin);
  caret = n = quote = 0;
  while(c != EOF && c != '\n') {
    if(!(caret+quote) && isblank(c))
      break;
    if(!caret && c == '"') {
      quote = quote ? 0 : 1;
      goto Loop;
    }
    if(!quote && c == '\'') {
      caret = caret ? 0 : 1;
      goto Loop;
    }
    if(!(caret+quote) && c == '\\')
      c = fgetc(stdin);
    if(n < len-1)
      buf[n++] = c;
  Loop:
    c = fgetc(stdin);
  }
  buf[n] = '\0';
  return n;
}

void
run(char **cmd) {
  char *ans, **p;
  int err, stat;
  FILE *tty;

  if(tflag)
    for(p = cmd; *p; p++) {
      if(p != cmd)
        fprint(stderr, " ");
      fprint(stderr, *p);
    }
  if(pflag) {
    fprint(stderr, "?...");
    tty = fopen("/dev/tty", "r+");
    if(tty == nil)
      fatal(1, "open /dev/tty: %m");
    ans = fgetln(tty);
    fclose(tty);
    if(*ans != 'y' && *ans != 'Y')
      return;
  } else if(tflag)
    fprint(stderr, "\n");
  if(fork() == 0) {
    execvp(*cmd, cmd);
    alert("exec %s: %m", *cmd);
    _exit(errno==ENOENT?127:126);
  }
  wait(&stat);
  err = WEXITSTATUS(stat);
  if(WIFSIGNALED(stat) || err == 255)
    exit(1);
  if(err == 126 || err == 127)
    exit(err);
  if(err)
    rval = err;
}

int
main(int argc, char *argv[]) {
  char *buf, **cmd, *eof;
  int cap, i, len, n, num, siz, total;
  int xflag;

  eof = nil;
  num = 0;
  siz = LINE_MAX;
  pflag = tflag = xflag = 0;
  ARGBEGIN("E:n:ps:tx"){
  case 'E':
    eof = optarg;
    break;
  case 'n':
    num = atoi(optarg);
    break;
  case 'p':
    pflag = 1;
    tflag = 1;
    break;
  case 's':
    siz = atoi(optarg);
    siz = min(siz, ARG_MAX-2048);
    break;
  case 't':
    tflag = 1;
    break;
  case 'x':
    xflag = 1;
    break;
  default:
    fprint(stderr, "usage: xargs [-ptx] [-E eofstr] [-n number]\n"
      "  [-s size] [utility [argument...]]\n");
    exit(1);
  }ARGEND

  cap = argc + INCR;
  cmd = malloc(cap*sizeof(char *));
  len = argc;
  for(i = 0; i < argc; i++) {
    cmd[i] = argv[i];
    siz -= strlen(argv[i]) + 1;
  }
  buf = malloc(siz);
  total = 0;
  while((n = getarg(buf, siz)) > 0) {
    if(eof && !strcmp(buf, eof))
      break;
    if(n == siz-1) {
      alert("argument too long");
      if(xflag)
        exit(1);
      continue;
    }
    if(total+n > siz || (num > 0 && len-argc == num)) {
      run(cmd);
      for(i = argc; i < len; i++)
        free(cmd[i]);
      len = argc;
      total = 0;
    }
    if(cap-1 == len) {
      cap += INCR;
      cmd = realloc(cmd, cap*sizeof(char *));
    }
    cmd[len++] = strdup(buf);
    cmd[len] = nil;
    total += n;
  }
  if(ferror(stdin))
    fatal(1, "read <stdin>: %m");
  if(len > argc)
    run(cmd);
  exit(rval);
}
