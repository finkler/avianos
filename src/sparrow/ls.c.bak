#include <u.h>
#include <avian.h>
#include <dirent.h>
#include <ftw.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>

#define INCR 100
#define sort(l) \
  (qsort((l)->f,(l)->len,sizeof(File),compar))

typedef struct {
  char *name;
  struct stat info;
} File;

typedef struct {
  File *f;
  int cap, len;
} FileList;

void addfile(FileList *, const char *, const struct stat *);
int alnumsort(const void *, const void *);
char *blocks(vlong);
void columnput(void);
struct stat *finfo(char *, int);
char *date(struct timespec *);
char *inodes(uvlong);
void listput(void);
void longput(void);
void ls(char *);
char mark(uint);
char *mode(uint);
FileList *newflist(void);
int nosort(const void *, const void *);
void setflist(FileList *);
int sizesort(const void *, const void *);
void putflist(void);
void putline(char *, ...);
void puttotal(void);
int timesort(const void *, const void *);
int walk(const char *, const struct stat *, int, struct FTW *);

int Aflag, Fflag, Hflag, Lflag, Rflag, aflag, cflag;
int dflag, iflag, kflag, lflag, nflag, pflag, qflag;
int rflag, sflag, uflag, xflag;
int fmt[2], rval;
int (*compar)(const void *, const void *);
void (*output)(void);
FileList *dl, *fl;

void
addfile(FileList *l, const char *s, const struct stat *sb) {
  ulong n;
  
  if(s == nil || sb == nil)
    return;
  if(l->cap == l->len) {
    l->cap += INCR;
    l->f = realloc(l->f, l->cap*sizeof(File));
  }
  memcpy(&l->f[l->len].info, sb, sizeof(struct stat));
  if(kflag) {
    n =  sb->st_blksize / 1024;
    if(n > 0) {
      l->f[l->len].info.st_blocks *= n;
    } else if(n == 0) {
      n = 1024 / sb->st_blksize;
      l->f[l->len].info.st_blocks /= n;
    }
  }
  l->f[l->len].name = strdup(s);
  l->len++;
}

int
alnumsort(const void *v1, const void *v2) {
  const File *f1, *f2;
  int r;

  f1 = (const File *)v1;
  f2 = (const File *)v2;
  r = strcoll(f1->name, f2->name);
  return rflag ? -r : r;
}

char *
blocks(vlong cnt) {
  static char s[21];
  int i, n;
  
  if(fmt[0] == 0)
    for(i = 0; i < fl->len; i++) {
      n = sprintf(s, "%llu", fl->f[i].info.st_blocks);
      if(fmt[0] < n)
        fmt[0] = n;
    }
  sprintf(s, "%*lld", fmt[0], cnt);
  return s;
}

void
columnput(void) {
  char buf[256];
  int i;
  uint len, n;
  
  len = 0;
  for(i = 0; i < fl->len; i++) {
    n = strlen(fl->f[i].name);
    if(len < n)
      len = n;
  }
  for(i = 0; i < fl->len; i++) {
    if(iflag)
      putline(inodes(fl->f[i].info.st_ino));
    if(sflag)
      putline(blocks(fl->f[i].info.st_blocks));
    strncpy(buf, fl->f[i].name, sizeof buf-2);
    if(Fflag || pflag) {
        n = strlen(buf);
        buf[n++] = mark(fl->f[i].info.st_mode);
        buf[n] = '\0';
    }
    putline("%*s", -len-1, buf);
  }
  putline(nil);
}

char *
date(struct timespec *ts) {
  static char s[13];
  long tdiff;

  tdiff = time(NULL)-ts->tv_sec;
  strftime(s, sizeof s, (tdiff >= 0 && tdiff < 15552000) ?
    "%b %e %H:%M" : "%b %e  %Y", localtime(&ts->tv_sec));
  return s;
}

struct stat *
finfo(char *s, int op) {
  static struct stat sb;
  
  if((Fflag+dflag+lflag) && !((Hflag*op)+Lflag) ?
    lstat(s, &sb) : stat(s, &sb)) {
    alert("can't stat %s: %m", s);
    rval = 1;
    return nil;
  }
  return &sb;
}

char *
inodes(uvlong ino) {
  static char s[21];
  int i, n;
  
  if(fmt[1] == 0)
    for(i = 0; i < fl->len; i++) {
      n = sprintf(s, "%llu", fl->f[i].info.st_ino);
      if(fmt[1] < n)
        fmt[1] = n;
    }
  sprintf(s, "%*llu", fmt[1], ino);
  return s;
}

void
listput(void) {
  int i;
  
  for(i = 0; i < fl->len; i++) {
    if(iflag)
      printf("%s ", inodes(fl->f[i].info.st_ino));
    if(sflag)
      printf("%s ", blocks(fl->f[i].info.st_blocks));
    print(fl->f[i].name);
    if(Fflag || pflag)
      printf("%c", mark(fl->f[i].info.st_mode));
    print("\n");
  }
}

void
longput(void) {
  char buf[256], *usr, *grp;
  int i, len[4], n;
  struct group *gr;
  struct passwd *pw;
  struct timespec *ts;
  
  memset(len, 0, 4*sizeof(int));
  for(i = 0; i < fl->len; i++) {
    n = sprintf(buf, "%u", fl->f[i].info.st_nlink);
    if(len[0] < n)
      len[0] = n;
    if(nflag) {
      n = sprintf(buf, "%d", fl->f[i].info.st_uid);
    } else {
      pw = getpwuid(fl->f[i].info.st_uid);
      n = pw ? strlen(pw->pw_name) : 4;
    }
    if(len[1] < n)
      len[1] = n;
    if(nflag) {
      n = sprintf(buf, "%d", fl->f[i].info.st_gid);
    } else {
      gr = getgrgid(fl->f[i].info.st_gid);
      n = gr ? strlen(gr->gr_name) : 4;
    }
    if(len[2] < n)
      len[2] = n;
    n = sprintf(buf, "%lld", fl->f[i].info.st_size);
    if(len[3] < n)
      len[3] = n;    
  }  
  for(i = 0; i < fl->len; i++) {
    if(iflag)
      printf("%s ", inodes(fl->f[i].info.st_ino));
    if(sflag)
      printf("%s ", blocks(fl->f[i].info.st_blocks));
    printf("%s %*u ", mode(fl->f[i].info.st_mode),
      len[0], fl->f[i].info.st_nlink);
    if(nflag) {
      printf("%*d %*d ", len[1], fl->f[i].info.st_uid,
        len[2], fl->f[i].info.st_gid);
    } else {
      pw = getpwuid(fl->f[i].info.st_uid);
      usr = pw ? pw->pw_name : "none";
      gr = getgrgid(fl->f[i].info.st_gid);
      grp = gr ? gr->gr_name : "none";
      printf("%*s %*s ", -len[1], usr, -len[2], grp);
    }
    if(cflag)
      ts = &fl->f[i].info.st_ctim;
    else if(uflag)
      ts = &fl->f[i].info.st_atim;
    else
      ts = &fl->f[i].info.st_mtim;
    printf("%*lld %s %s", len[3], fl->f[i].info.st_size,
      date(ts), fl->f[i].name);
    if(S_ISLNK(fl->f[i].info.st_mode)) {
      n = readlink(fl->f[i].name, buf, sizeof buf-1);
      if(n > 0) {
        buf[n] = '\0';
        printf(" -> %s", buf);
      }
    } else if(Fflag || pflag)
      printf("%c", mark(fl->f[i].info.st_mode));
    print("\n");
  }
}

void
ls(char *s) {
  struct stat *sb;
  
  if(!(sb = finfo(s, 1)))
    return;
  if(dflag || !S_ISDIR(sb->st_mode)) {
    addfile(fl, s, sb);
    return;
  }
  if(Rflag) {
    if(nftw(s, walk, 20, Lflag ? 0 : FTW_PHYS) < 0) {
      alert("nftw %s: %m", s);
      rval = 1;
    }
  } else {
    addfile(dl, s, sb);  
  }
}

char
mark(uint m) {
  if(S_ISDIR(m))
    return '/';
  if(pflag)
    return '\0';
  if(S_ISFIFO(m))
    return '|';
  if(S_ISLNK(m))
    return '@';
  if(m&(S_IXUSR|S_IXGRP|S_IXOTH))
    return '*';
  return '\0';
}

char *
mode(uint m) {
  static char s[11];

  memset(s, '-', sizeof s);
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
  if(m & S_ISUID) {
    if(m & S_IXUSR)
      s[3] = 's';
    else
      s[3] = 'S';
  } else if(m & S_IXUSR)
    s[3] = 'x';
  if(m & S_IRGRP)
    s[4] = 'r';
  if(m & S_IWGRP)
    s[5] = 'w';
  if(m & S_ISGID) {
    if(m & S_IXGRP)
      s[6] = 's';
    else
      s[6] = 'S';
  } else if(m & S_IXGRP)
    s[6] = 'x';
  if(m & S_IROTH)
    s[7] = 'r';
  if(m & S_IWOTH)
    s[8] = 'w';
  if(m & S_IXOTH)
    s[9] = 'x';
  s[10] = '\0';
  return s;
}

FileList *
newflist(void) {
  FileList *l;
  
  l = malloc(sizeof(FileList));
  l->cap = INCR;
  l->len = 0;
  l->f = malloc(l->cap*sizeof(File));
  return l;
}

int
nosort(const void *v1, const void *v2) {
  return 0;
}

void
putflist(void) {
  int i;
  char *p;

  if(fl->len == 0)
    return;
  if(isatty(1) || qflag)
    for(i = 0; i < fl->len; i++)
      for(p = fl->f[i].name; *p; p++)
        if(!isprint(*p) || *p == '\t')
          *p = '?';
  fmt[0] = fmt[1] = 0;
  output();
}

void
putline(char *fmt, ...) {
  static int len, tw;
  va_list ap;
  char buf[LINE_MAX+1];
  int n;

  if(tw == 0)
    tw = textwidth();
  if(fmt == nil) {
    print("\n");
    return;
  }
  va_start(ap, fmt);
  n = vsnprintf(buf, LINE_MAX, fmt, ap);
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

void
puttotal(void) {
  int i;
  vlong total;
  
  total = 0;
  for(i = 0; i < fl->len; i++)
    total += fl->f[i].info.st_blocks;
  printf("total %lld\n", total);
}

void
setflist(FileList *l) {
  int i;
  
  for(i = 0; i < l->len; i++)
    free(l->f[i].name);
  l->len = 0;
}

int
sizesort(const void *v1, const void *v2) {
  const File *f1, *f2;
  vlong odiff;
  int r;

  f1 = (const File *)v1;
  f2 = (const File *)v2;
  r = (odiff = f1->info.st_size - f2->info.st_size) ?
    odiff : strcoll(f1->name, f2->name);
  return rflag ? -r : r;
}

/* strcpy critical here */
void
streamput(void) {
  char buf[LINE_MAX+1], *p;
  int i, n;
  
  for(i = 0; i < fl->len; i++) {
    p = buf;
    if(iflag)
      p += sprintf(p, "%llu ", fl->f[i].info.st_ino);
    if(sflag)
      p += sprintf(p, "%lld ", fl->f[i].info.st_blocks);
    n = p - buf;
    strncpy(p, fl->f[i].name, LINE_MAX-n-2); // check len
    p += strlen(p);
    if(Fflag)
      *p++ = mark(fl->f[i].info.st_mode);
    *p = '\0';
    if(i == fl->len-1)
      putline("%s", buf);
    else
      putline("%s,", buf);
  }
  putline(nil);
}

int
timesort(const void *v1, const void *v2) {
  const File *f1, *f2;
  long tdiff, t1, t2;
  int r;

  f1 = (const File *)v1;
  f2 = (const File *)v2;
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
  r = (tdiff = t1 - t2) ? tdiff : strcoll(f1->name, f2->name);
  return rflag ? -r : r;
}

int
walk(const char *fpath, const struct stat *sb, int tflag,
  struct FTW *ftwbuf) {
  if(tflag == FTW_D)
    addfile(dl, fpath, sb);
  return 0;
}

int
main(int argc, char *argv[]) {
  DIR *d;
  struct dirent *ent;
  int i, n;
  
  Aflag = Fflag = Hflag = Lflag = Rflag = aflag =
  cflag = dflag = iflag = kflag = lflag = nflag =
  pflag = qflag = rflag = sflag = uflag = xflag = 0;
  compar = alnumsort;
  output = listput;
  rval = 0;
  ARGBEGIN("ACFHLRSacdfiklmnpqrstux1"){
  case 'A':
    Aflag = 1;
    break;
  case 'C':
    output = columnput;
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
    output = longput;
    break;
  case 'm':
    output = streamput;
    break;
  case 'n':
    lflag = 1;
    nflag = 1;
    output = longput;
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
    output = columnput;
    xflag = 1;
    break;
  case '1':
    output = listput;
    break;
  default:
    fprint(stderr,
      "usage: ls [-ACFRSacdfiklmnpqrstux1] [-H|-L] [file...]\n");
    exit(1);
  }ARGEND

  dl = newflist();
  fl = newflist();
  if(argc == 0)
    ls(".");
  for(i = 0; i < argc; i++)
      ls(argv[i]);
  sort(fl);
  putflist();
  sort(dl);
  n = fl->len;
  for(i = 0; i < dl->len; i++) {
    d = opendir(dl->f[i].name);
    if(d == nil) {
      alert("can't open %s: %m", dl->f[i].name);
      rval = 1;
      continue;
    }
    if(chdir(dl->f[i].name)) {
      alert("error reading %s: %m", dl->f[i].name);
      rval = 1;
      continue;
    }
    setflist(fl);
    while((ent = readdir(d))) {
      if((!(Aflag+aflag) && ent->d_name[0] == '.') ||
        (Aflag && (!strcmp(ent->d_name, ".") ||
        !strcmp(ent->d_name, ".."))))
        continue;      
      addfile(fl, ent->d_name, finfo(ent->d_name, 0));
    }
    closedir(d);
    if((dl->len+n) > 1) {
      if(n == 0) {
        if(i > 0)
          print("\n");        
      } else {
        print("\n");
      }
      printf("%s:\n", dl->f[i].name);
    }
    if(lflag+nflag+sflag)
      puttotal();
    sort(fl);
    putflist();
  }
  exit(rval);
}
