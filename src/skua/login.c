#include <u.h>
#include <avian.h>
#include <pwd.h>
#include <sechash.h>
#include <shadow.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>

#define PATH "/bin"
#define TERM "vt100"

struct passwd *pwd;

void
auth(char *user)
{
  char *p, *pass;
  struct spwd *sp;
  SHA1Digest *digest;

  fprint(stderr, "\n");
  if(user == nil)
    user = readcons("user", nil, 0);
  pwd = getpwnam(user);
  if(pwd == nil)
    fatal(1, "unknown user %s", user);
  sp = getspnam(pwd->pw_name);
  if(sp != nil){
    pass = readcons("password", nil, 1);
    digest = sha1(pass, strlen(pass), nil);
    p = sha1pickle(digest);
    if(strcmp(sp->sp_pwdp, p))
      fatal(1, "wrong password");
    free(digest);
    free(p);
  }
  fprint(stderr, "\n");
}

void
setup(void)
{
  fchown(0, pwd->pw_uid, pwd->pw_gid);
  fchmod(0, 0600);
  setenv("HOME", pwd->pw_dir, 1);
  setenv("LOGNAME", pwd->pw_name, 1);
  setenv("PATH", PATH, 0);
  setenv("TERM", TERM, 0);
  setgid(pwd->pw_gid);
  setuid(pwd->pw_uid);
  if(chdir(pwd->pw_dir)){
    alert("chdir %s: %m", pwd->pw_dir);
    if(chdir("/"))
      exit(1);
  }
}

void
usage(void)
{
  fprint(stderr, "usage: login [user]\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  ARGBEGIN(""){
  default:
    usage();
  }ARGEND

  if(argc > 1)
    usage();
  if(!isatty(0))
    fatal(1, "not a tty");
  setpriority(PRIO_PROCESS, 0, 0);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  auth(argv[0]);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  signal(SIGTSTP, SIG_IGN);
  setup();
  execl(pwd->pw_shell, pwd->pw_shell, nil);
  fatal(1, "exec %s: %m", pwd->pw_shell);
}
