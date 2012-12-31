#include <u.h>
#include <avian.h>
#include <dirent.h>
#include <ftw.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>

typedef struct File File;
struct File {
  char *name;
  struct stat info;
  File *next;
};

void allot(char *);
int alnumsort(File *, File *);
void clear(File *);
struct stat *infoat(int, char *);
void insert(File **, char *, struct stat *);
void list(File *);
int nosort(File *, File *);
void scan(char *);
int sort(File *, File *);
int sizesort(File *, File *);
int timesort(File *, File *);

int Aflag, Fflag, Hflag, Lflag, Rflag, aflag, cflag;
int dflag, iflag, kflag, lflag, nflag, pflag, qflag;
int rflag, sflag, uflag, xflag;
int rval;
int (*compar)(File *, File *);
File *dir, *f;

void
allot(char *s) {
  struct stat *sb;
  
  sb = infoat(AT_FDCWD, s);
  if(sb == nil)
    return;
  if(dflag || !S_ISDIR(sb->st_mode))
    insert(&f, s, sb);
  else
    insert(&dir, s, sb);
}

int
alnumsort(File *f1, File *f2) {
  return strcoll(f1->name, f2->name);
}

void
clear(File *l) {
  File *r1, *r2;

  r1 = l;
  while(r1) {
    r2 = r1->next;
    free(r1->name);
    free(r1);
    r1 = r2;
  }
}

struct stat *
infoat(int fd, char *s) {
  static struct stat sb;

  if(fstatat(fd, s, &sb, AT_SYMLINK_NOFOLLOW)) {
    alert("can't stat %s: %m", s);
    rval = 1;
    return nil;
  }
  return &sb;
}

void
insert(File **l, char *s, struct stat *sb) {
  File *new, *r;

  new = malloc(sizeof(File));
  new->name = strdup(s);
  new->info = *sb;
  if(*l == nil || sort(*l, new) > 0) {
    new->next = nil;
    *l = new;
  } else {
    r = *l;
    while(r->next && sort(r, new) < 0)
      r = r->next;
    new->next = r->next;
    r->next = new;
  }
}

void
list(File *l) {
  File *r;

  for(r = l; r; r = r->next)
    println(r->name);
}

int
nosort(File *f1, File *f2) {
  return 0;
}

void
scan(char *s) {
  DIR *d;
  struct dirent *ent;
  int fd;
  File *l, *r;
  char *p;
  struct stat *sb;

  d = opendir(s);
  if(d == nil) {
    alert("opendir");
    rval = 1;
    return;
  }
  fd = dirfd(d);
  l = nil;
  while((ent = readdir(d))) {
    sb = infoat(fd, ent->d_name);
    if(sb == nil)
      continue;
    insert(&l, ent->d_name, sb);
  }
  closedir(d);
  list(l);
  for(r = l; Rflag && r; r = r->next)
    if(S_ISDIR(r->info.st_mode) && strcmp(r->name, ".") &&
      strcmp(r->name, "..")) {
      p = stradd(s, "/", r->name);
      printf("\n%s:\n", p);
      scan(p);
      free(p);
    }
  clear(l);
}

int
sizesort(File *f1, File *f2) {
  vlong odiff;

  return (odiff = f1->info.st_size - f2->info.st_size) ?
    odiff : strcoll(f1->name, f2->name);
}

int
sort(File *f1, File *f2) {
  int r;

  r = compar(f1, f2);
  return rflag ? -r : r;
}

int
timesort(File *f1, File *f2) {
  long tdiff, t1, t2;

  if(cflag) {
    t1 = f1->info.st_ctim.tv_sec;
    t2 = f2->info.st_ctim.tv_sec;
  } else if(uflag) {
    t1 = f1->info.st_atim.tv_sec;
    t2 = f2->info.st_atim.tv_sec;
  } else {
    t1 = f1->info.st_mtim.tv_sec;
    t2 = f2->info.st_mtim.tv_sec;
  }
  return (tdiff = t1 - t2) ? tdiff : strcoll(f1->name, f2->name);
}

int
main(int argc, char *argv[]) {
  int i;
  File *r;
  
  Aflag = Fflag = Hflag = Lflag = Rflag = aflag =
  cflag = dflag = iflag = kflag = lflag = nflag =
  pflag = qflag = rflag = sflag = uflag = xflag = 0;
  compar = alnumsort;
  rval = 0;
  ARGBEGIN("ACFHLRSacdfiklmnpqrstux1"){
  case 'A':
    Aflag = 1;
    break;
  case 'C':
    //output = columnput;
    xflag = 0;
    break;
  case 'F':
    Fflag = 1;
    break;
  case 'H':
    Hflag = 1;
    Lflag = 0;
    break;
  case 'L':
    Lflag = 1;
    Hflag = 0;
    break;
  case 'R':
    Rflag = 1;
    break;
  case 'S':
    compar = sizesort;
    break;
  case 'a':
    aflag = 1;
    break;
  case 'c':
    cflag = 1;
    uflag = 0;
    break;
  case 'd':
    dflag = 1;
    break;
  case 'f':
    aflag = 1;
    rflag = sflag = 0;
    compar = nosort;
    break;
  case 'i':
    iflag = 1;
    break;
  case 'k':
    kflag = 1;
    break;
  case 'l':
    lflag = 1;
    nflag = 0;
    if(compar == nosort)
      compar = alnumsort;
    //output = longput;
    break;
  case 'm':
    //output = streamput;
    break;
  case 'n':
    lflag = 1;
    nflag = 1;
    //output = longput;
    break;
  case 'p':
    pflag = 1;
    break;
  case 'q':
    qflag = 1;
    break;
  case 'r':
    rflag = 1;
    break;
  case 's':
    sflag = 1;
    break;
  case 't':
    compar = timesort;
    break;
  case 'u':
    cflag = 0;
    uflag = 1;
    break;
  case 'x':
    //output = columnput;
    xflag = 1;
    break;
  case '1':
    //output = listput;
    break;
  default:
    fprint(stderr,
      "usage: ls [-ACFRSacdfiklmnpqrstux1] [-H|-L] [file...]\n");
    exit(1);
  }ARGEND

  if(argc == 0)
    allot(".");
  for(i = 0; i < argc; i++)
      allot(argv[i]);
  list(f);
  r = dir;
  if(r && (r->next || f || Rflag))
    printf("%s:\n", r->name);
  clear(f);
  while(r) {
    scan(r->name);
    if(r->next)
      printf("\n%s:\n", r->name);
    r = r->next;
  }
  exit(rval);
}
