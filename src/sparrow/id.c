#include <u.h>
#include <avian.h>
#include <grp.h>
#include <pwd.h>

#define INCR 10

typedef struct {
  char *name;
  int gid;
} Group;

int Gflag, gflag, nflag, rflag, uflag;
int euid, ruid, egid, rgid;
char *eusr, *rusr;

Group *
findgroups(char *s) {
  int cap, len;
  Group *g;
  struct group *grp;
  char **r;  
  
  cap = INCR;
  len = 1;
  g = malloc(cap*sizeof(Group));
  setgrent();
  while((grp = getgrent()))
    if(grp->gr_gid == rgid) {
      g[0].name = strdup(grp->gr_name);
      g[0].gid = grp->gr_gid;
    } else if(grp->gr_gid == egid) {
      g[1].name = strdup(grp->gr_name);
      g[1].gid = grp->gr_gid;
      len++;
    }
  setgrent();
  while((grp = getgrent())) {
    if(grp->gr_gid == rgid || grp->gr_gid == egid)
      continue;
    for(r = grp->gr_mem; *r; r++)
      if(!strcmp(s, *r)) {
        if(cap == len-1) {
          cap += INCR;
          g = realloc(g, cap*sizeof(Group));
        }
        g[len].name = strdup(grp->gr_name);
        g[len].gid = grp->gr_gid;
        len++;
        break;
      }
  }
  endgrent();
  g[len].name = nil;
  g[len].gid = 0;
  return g;
}

void
usage(void) {
	fprint(stderr, "usage: id [user]\n"
    "       id -G [-n] [user]\n"
    "       id -g [-nr] [user]\n"
    "       id -u [-nr] [user]\n");
	exit(1);
}

int
main(int argc, char *argv[]) {
  struct passwd *pwd;
  struct group *grp;
  Group *grps, *r;
  int i, j;
  
  Gflag = gflag = nflag = rflag = uflag = 0;
	ARGBEGIN("Ggnru"){
	case 'G':
    Gflag = 1;
		break;
	case 'g':
    gflag = 1;
		break;
	case 'n':
    nflag = 1;
		break;
	case 'r':
    rflag = 1;
		break;
	case 'u':
     uflag = 1;
    break;
	default:
		usage();
	}ARGEND

	if(argc > 1 || (Gflag+gflag+uflag) > 1 || (Gflag+rflag) > 1 ||
    (!(Gflag+gflag+uflag) && (nflag+rflag)))
    usage();
  if(argc == 1) {
    pwd = getpwnam(argv[0]);
    if(pwd == nil)
      fatal(1, "unknown user %s", argv[0]);
    eusr = rusr = strdup(pwd->pw_name);
    euid = ruid = pwd->pw_uid;
    egid = rgid = pwd->pw_gid;
  } else {
    eusr = rusr = getlogin();
    euid = geteuid();
    ruid = getuid();
    egid = getegid();
    rgid = getgid();
    if(euid != ruid) {
      pwd = getpwuid(euid);
      if(pwd == nil)
        fatal(1, "unknown user id %d", euid);
      eusr = strdup(pwd->pw_name);
    }
  }
  if(uflag) {
    if(nflag)
      printf("%s\n", rflag ? rusr : eusr);
    else
      printf("%d\n", rflag ? ruid : euid);
    exit(0);
    /* not reached */
  }
  if(gflag) {
    if(nflag) {
      grp = getgrgid(rflag ? rgid : egid);
      if(grp == nil)
        fatal(1, "unknown group id %d", rflag ? rgid : egid);
      printf("%s\n", grp->gr_name);
    } else {
      printf("%d\n", rflag ? rgid : egid);
    }
    exit(0);
    /* not reached */
  }
  grps = findgroups(rusr);
  if(Gflag) {
    for(r = grps; r->name; r++) {
      if(r != grps)
        print(" ");
      if(nflag)
        printf("%s", r->name);
      else
        printf("%d", r->gid);
    }
    print("\n");
    exit(0);
    /* not reached */
  }
  printf("uid=%d(%s) gid=%d(%s)",
    ruid, rusr, grps[0].gid, grps[0].name);
  if(euid != ruid)
    printf(" euid=%d(%s)", euid, eusr);
  i = 1;
  if(egid != rgid) {
    printf(" eguid=%d(%s)", grps[i].gid, grps[i].name);
    i++;
  }
  for(j = 0; grps[i].name; i++, j++) {
    if(j == 0)
      print(" groups=");
    else
      print(",");
    printf("%d(%s)", grps[i].gid, grps[i].name);
  }
  print("\n");
}
