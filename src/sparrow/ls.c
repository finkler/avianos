#include <u.h>
#include <avian.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <utf8.h>

enum
{
  FmtBlk,
  FmtGrp,
  FmtLnk,
  FmtIno,
  FmtNam,
  FmtSiz,
  FmtUsr,
  FmtLast
};

typedef struct File File;
struct File
{
  char        *name;
  struct stat info;
  File        *next;
};

typedef struct Visitor Visitor;
struct Visitor
{
  struct stat *info;
  Visitor     *next;
};

int  alnumsort(File*, File*);
void assign(char*);
void clear(File*);
struct stat*
     fileinfo(char*, int);
void insert(File**, char*, struct stat*);
int  nosort(File*, File*);
uint longlen(vlong);
void popvisitor(void);
void printcolumn(File*);
void printdate(struct timespec*);
void printlong(File*);
void printmark(uint, int);
void printmode(uint);
void printname(char*);
void printone(File*);
void printstream(File*);
void pushvisitor(struct stat*);
void show(File*);
int  sizesort(File*, File*);
int  sort(File*, File*);
int  timesort(File*, File*);
void visit(char*, struct stat*);
int  wasvisited(struct stat*);

int Aflag;
int Fflag;
int Hflag;
int Lflag;
int Rflag;
int aflag;
int cflag;
int dflag;
int iflag;
int kflag;
int lflag;
int nflag;
int pflag;
int qflag;
int rflag;
int sflag;
int uflag;
int xflag;

int  (*compar)(File*, File*);
int  dfd;
File *dir;
File *f;
int  field[FmtLast];
void (*output)(File*);

Visitor *v;

int
alnumsort(File *f1, File *f2)
{
  return strcoll(f1->name, f2->name);
}

void
assign(char *s)
{
  struct stat *sb;

  sb = fileinfo(s, 1);
  if(sb == nil)
    return;
  if(dflag || !S_ISDIR(sb->st_mode))
    insert(&f, s, sb);
  else
    insert(&dir, s, sb);
}

void
clear(File *l)
{
  File *r1, *r2;

  r1 = l;
  while(r1){
    r2 = r1->next;
    free(r1->name);
    free(r1);
    r1 = r2;
  }
}

struct stat*
fileinfo(char *s, int op)
{
  static struct stat sb;
  int flags;

  flags = 0;
  // TODO: right logic?
  if((Fflag+dflag+lflag) && !((Hflag*op)+Lflag))
    flags = AT_SYMLINK_NOFOLLOW;
  if(dfd < 0)
    dfd = AT_FDCWD;
  if(fstatat(dfd, s, &sb, flags)){
    alert("stat %s: %m", s);
    return nil;
  }
  return &sb;
}

void
insert(File **l, char *s, struct stat *sb)
{
  File *new, *r;

  if(s == nil || sb == nil)
    return;
  new = malloc(sizeof(File));
  new->name = strdup(s);
  new->info = *sb;
  if(*l == nil || sort(*l, new) > 0){
    new->next = *l;
    *l = new;
  }else{
    r = *l;
    while(r->next && sort(r->next, new) < 0)
      r = r->next;
    new->next = r->next;
    r->next = new;
  }
}

int
nosort(File *f1, File *f2)
{
  return 0;
}

void
popvisitor(void)
{
  Visitor *r;

  if(v == nil)
    return;
  r = v->next;
  free(v);
  v = r;
}

void
printcolumn(File *l)
{
  int cnt, cols, i, j, k, n;
  int ncols, nrows, sp;
  File *r, *p;
  char *s;

  memset(&fmt, 0, sizeof fmt);
  for(cnt = 0, r = l; r; cnt++, r = r->next){
    if(iflag){
      n = longlen(r->info.st_ino);
      if(n > field[FmtIno])
        field[FmtIno] = n;
    }
    if(sflag){
      n = longlen(r->info.st_blocks);
      if(n > field[FmtBlk])
        field[FmtBlk] = n;
    }
    n = utflen(r->name);
    if(n > field[FmtNam])
      field[FmtNam] = n;
  }
  sp = 0;
  if(Fflag || pflag)
    sp++;
  if(iflag)
    sp++;
  if(sflag)
    sp++;
  cols = textwidth();
  n = field[FmtIno] + field[FmtBlk] + field[FmtNam] + sp;
  ncols = cols / n;
  if(cols%n < (ncols-1)*2)
    ncols--;
  if(ncols < 1)
    ncols = 1;
  nrows = cnt / ncols;
  if(cnt%ncols)
    nrows++;
  r = l;
  for(i = 0; i < nrows; i++){
    p = r;
    for(j = 0; j < ncols; j++){
      if(j > 0)
        print("  ");
      if(iflag)
        printf("%*lld ", field[FmtIno], p->info.st_ino);
      if(sflag)
        printf("%*llu ", field[FmtBlk], p->info.st_blocks);
      printname(p->name);
      if(Fflag || pflag)
        printmark(p->info.st_mode, 1);
      s = p->name;
      for(k = 0; k < nrows && p; k++){
        p = p->next;
        if(xflag)
          break;
      }
      if(p == nil)
        break;
      n = field[FmtNam] - utflen(s);
      while(n--)
        print(" ");
    }
    print("\n");
    if(xflag)
      r = p;
    else
      r = r->next;
  }
}

void
printdate(struct timespec *ts)
{
  static char s[13];
  long tdiff;

  tdiff = time(NULL)-ts->tv_sec;
  strftime(s, sizeof s, (tdiff>=0 && tdiff<15552000) ?
    "%b %e %H:%M":"%b %e  %Y", localtime(&ts->tv_sec));
  print(s);
}

void
printlong(File *l)
{
  static char buf[PATH_MAX];
  int n;
  File *r;
  struct passwd *usr;
  struct group *grp;

  memset(&fmt, 0, sizeof fmt);
  for(r = l; r; r = r->next){
    if(iflag){
      n = longlen(r->info.st_ino);
      if(n > field[FmtIno])
        field[FmtIno] = n;
    }
    if(sflag){
      n = longlen(r->info.st_blocks);
      if(n > field[FmtBlk])
        field[FmtBlk] = n;
    }
    n = longlen(r->info.st_nlink);
    if(n > field[FmtLnk])
      field[FmtLnk] = n;
    n = longlen(r->info.st_rdev ?
      r->info.st_rdev : r->info.st_size);
    if(n > field[FmtSiz])
      field[FmtSiz] = n;
    usr = nil;
    grp = nil;
    if(!nflag){
      usr = getpwuid(r->info.st_uid);
      grp = getgrgid(r->info.st_gid);
    }
    n = usr ? utflen(usr->pw_name) : longlen(r->info.st_uid);
    if(n > field[FmtUsr])
      field[FmtUsr] = n;
    n = grp ? utflen(grp->gr_name) : longlen(r->info.st_gid);
    if(n > field[FmtGrp])
      field[FmtGrp] = n;
  }
  for(r = l; r; r = r->next){
    if(iflag)
      printf("%*lld ", field[FmtIno], r->info.st_ino);
    if(sflag)
      printf("%*llu ", field[FmtBlk], r->info.st_blocks);
    printmode(r->info.st_mode);
    printf(" %*u", field[FmtLnk], r->info.st_nlink);
    if(nflag){
      printf(" %*u %*u", field[FmtUsr], r->info.st_uid,
        field[FmtGrp], r->info.st_gid);
    }else{
      usr = getpwuid(r->info.st_uid);
      if(usr)
        printf(" %*s", field[FmtUsr], usr->pw_name);
      else
        printf(" %*u", field[FmtUsr], r->info.st_uid);
      grp = getgrgid(r->info.st_gid);
      if(grp)
        printf(" %*s", field[FmtGrp], grp->gr_name);
      else
        printf(" %*u", field[FmtGrp], r->info.st_gid);
    }
    printf(" %*llu ", field[FmtSiz], r->info.st_rdev ?
      r->info.st_rdev : r->info.st_size);
    printdate(&r->info.st_mtim);
    print(" ");
    printname(r->name);
    if(S_ISLNK(r->info.st_mode)
    && readlinkat(dfd, r->name, buf, PATH_MAX) > 0)
      printf(" -> %s", buf);
    else if(Fflag || pflag)
      printmark(r->info.st_mode, 0);
    print("\n");
  }
}

void
printmark(uint m, int fill)
{
  if(S_ISDIR(m))
    print("/");
  else if(pflag)
    goto End;
  else if(S_ISFIFO(m))
    print("|");
  else if(S_ISLNK(m))
    print("@");
  else if(S_ISSOCK(m))
    print("=");
  else if(m&(S_IXUSR|S_IXGRP|S_IXOTH))
    print("*");
  else{
  End:
    if(fill)
      print(" ");
  }
}

void
printmode(uint m)
{
  static char s[11];

  memset(s, '-', 10);
  if(S_ISBLK(m))
    s[0] = 'b';
  else if(S_ISCHR(m))
    s[0] = 'c';
  else if(S_ISDIR(m))
    s[0] = 'd';
  else if(S_ISFIFO(m))
    s[0] = 'p';
  else if(S_ISLNK(m))
    s[0] = 'l';
  if(m & S_IRUSR)
    s[1] = 'r';
  if(m & S_IWUSR)
    s[2] = 'w';
  if(m & S_ISUID){
    if(m & S_IXUSR)
      s[3] = 's';
    else
      s[3] = 'S';
  }else if(m & S_IXUSR)
    s[3] = 'x';
  if(m & S_IRGRP)
    s[4] = 'r';
  if(m & S_IWGRP)
    s[5] = 'w';
  if(m & S_ISGID){
    if(m & S_IXGRP)
      s[6] = 's';
    else
      s[6] = 'S';
  }else if(m & S_IXGRP)
    s[6] = 'x';
  if(m & S_IROTH)
    s[7] = 'r';
  if(m & S_IWOTH)
    s[8] = 'w';
  if(m & S_IXOTH)
    s[9] = 'x';
  print(s);
}

void
printname(char *s)
{
  char *p;

  if(qflag)
    for(p = s; *p; p++)
      if(!isprint(*p) || *p == '\t')
        *p = '?';
  print(s);
}

void
printone(File *l)
{
  int n;
  File *r;

  memset(&fmt, 0, sizeof fmt);
  if(iflag+sflag)
    for(r = l; r; r = r->next){
      if(iflag){
        n = longlen(r->info.st_ino);
        if(n > field[FmtIno])
          field[FmtIno] = n;
      }
      if(sflag){
        n = longlen(r->info.st_blocks);
        if(n > field[FmtBlk])
          field[FmtBlk] = n;
      }
    }
  for(r = l; r; r = r->next){
    if(iflag)
      printf("%*lld ", field[FmtIno], r->info.st_ino);
    if(sflag)
      printf("%*llu ", field[FmtBlk], r->info.st_blocks);
    printname(r->name);
    if(Fflag || pflag)
      printmark(r->info.st_mode, 0);
    print("\n");
  }
}

void
printstream(File *l)
{
  int cols, len, n;
  File *r;

  cols = columns();
  len = 0;
  for(r = l; r; r = r->next){
    n = 0;
    if(iflag)
      n += longlen(r->info.st_ino) + 1;
    if(sflag)
      n += longlen(r->info.st_blocks) + 1;
    n += utflen(r->name);
    if(Fflag || pflag)
      n++;
    if(r->next)
      n++;
    if(r != l){
      n++;
      if(len + n > cols){
        len = 0;
        print("\n");
      }else
        print(" ");
    }
    if(iflag)
      printf("%lld ", r->info.st_ino);
    if(sflag)
      printf("%llu ", r->info.st_blocks);
    printname(r->name);
    if(Fflag || pflag)
      printmark(r->info.st_mode, 0);
    if(r->next)
      print(",");
    else
      print("\n");
    len += n;
  }
}


void
pushvisitor(struct stat *sb)
{
  Visitor *new;

  new = malloc(sizeof(Visitor));
  new->info = sb;
  new->next = v;
  v = new;
}

void
show(File *l)
{
  File *r;
  uvlong total;

  if(l == nil)
    return;

  if(lflag+nflag+sflag){
    total = 0;
    for(r = l; r; r = r->next)
      total += r->info.st_blocks;
    printf("total %llu\n", total);
  }
  output(l);
}

int
sizesort(File *f1, File *f2)
{
  vlong odiff;

  return (odiff=f2->info.st_size-f1->info.st_size) ?
    odiff : strcoll(f1->name, f2->name);
}

int
sort(File *f1, File *f2)
{
  int r;

  r = compar(f1, f2);
  return rflag ? -r : r;
}

int
timesort(File *f1, File *f2)
{
  long tdiff, t1, t2;

  if(cflag){
    t1 = f1->info.st_ctim.tv_sec;
    t2 = f2->info.st_ctim.tv_sec;
  }else if(uflag){
    t1 = f1->info.st_atim.tv_sec;
    t2 = f2->info.st_atim.tv_sec;
  }else{
    t1 = f1->info.st_mtim.tv_sec;
    t2 = f2->info.st_mtim.tv_sec;
  }
  return (tdiff=t2-t1) ?
    tdiff : strcoll(f1->name, f2->name);
}

void
visit(char *s, struct stat *sb)
{
  DIR *d;
  struct dirent *ent;
  File *l, *r;
  char *p;

  if(wasvisited(sb)){
    alert("%s: already listed", s);
    return;
  }
  pushvisitor(sb);
  d = opendir(s);
  if(d == nil){
    alert("open %s: %m", s);
    return;
  }
  dfd = dirfd(d);
  l = nil;
  while((ent = readdir(d))){
    if((!(Aflag+aflag) && ent->d_name[0] == '.')
    || (Aflag && (!strcmp(ent->d_name, ".")
    || !strcmp(ent->d_name, ".."))))
      continue;
    insert(&l, ent->d_name, fileinfo(ent->d_name, 0));
  }
  show(l);
  closedir(d);
  dfd = -1;
  for(r = l; Rflag && r; r = r->next)
    if(S_ISDIR(r->info.st_mode) && strcmp(r->name, ".")
    && strcmp(r->name, "..")){
      p = stradd(s, "/", r->name);
      printf("\n%s:\n", p);
      visit(p, &r->info);
      free(p);
    }
  clear(l);
  popvisitor();
}

int
wasvisited(struct stat *sb)
{
  Visitor *r;

  for(r = v; r; r = r->next)
    if(r->info->st_dev == sb->st_dev
    && r->info->st_ino == sb->st_ino)
      return 1;
  return 0;
}

int
main(int argc, char *argv[])
{
  int i;
  File *r;

  Aflag = Fflag = Hflag = Lflag = Rflag = aflag =
  cflag = dflag = iflag = kflag = lflag = nflag =
  pflag = qflag = rflag = sflag = uflag = xflag = 0;
  compar = alnumsort;
  output = printone;
  ARGBEGIN("ACFHLRSacdfiklmnpqrstux1"){
  case 'A':
    Aflag = 1;
    break;
  case 'C':
    output = printcolumn;
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
    output = printlong;
    break;
  case 'm':
    output = printstream;
    break;
  case 'n':
    lflag = 1;
    nflag = 1;
    output = printlong;
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
    output = printcolumn;
    xflag = 1;
    break;
  case '1':
    output = printone;
    break;
  default:
    fprint(stderr,
      "usage: ls [-ACFRSacdfiklmnpqrstux1] [-H|-L] [file...]\n");
    exit(1);
  }ARGEND

  if(isatty(1))
    qflag = 1;
  dfd = -1;
  if(argc == 0)
    assign(".");
  for(i = 0; i < argc; i++)
    assign(cleanname(argv[i]));
  show(f);
  if(dir && (dir->next || f || Rflag))
    printf("%s:\n", dir->name);
  clear(f);
  for(r = dir; r; r = r->next){
    visit(r->name, &r->info);
    if(r->next)
      printf("\n%s:\n", r->next->name);
  }
  exit(rval);
}
