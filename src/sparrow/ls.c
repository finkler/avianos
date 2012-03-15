#include <avian.h>
#include <dirent.h>
#include <ftw.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>

#define LINCR 100
#define TEXTWIDTH 80

typedef struct {
  char *name;
  struct stat info;
} File;

typedef struct {
  File *f;
  int cap, len;
} FileList;

enum { BlockFormat, GroupFormat, LinkFormat, OwnerFormat, SerialFormat, SizeFormat, LastFormat };
enum { ColumnPut, ListPut, LongPut, StreamPut, LastPut };
enum { AlnumSort, NoSort, SizeSort, TimeSort, LastSort };

void addfile(FileList *, File *);
int alnumsort(const void *, const void *);
void columnput(FileList *);
char *date(struct timespec *);
char *filename(File *);
void initlist(FileList *);
void listput(FileList *);
void ls(char *);
char mark(mode_t);
char *mode(mode_t);
int nosort(const void *, const void *);
void output(FileList *);
char *prefix(struct stat *); // <-- 
void longput(FileList *);

void setformat(FileList *); // <-- void settextwidth(void);
int sizesort(const void *, const void *);
void sortlist(FileList *);
void streamput(FileList *);
int timesort(const void *, const void *);
int walk(const char *, const struct stat *, int, struct FTW *);

int Aflag, Cflag, Fflag, Hflag, Lflag, Rflag, Sflag;
int aflag, cflag, dflag, fflag, iflag, kflag, lflag;
int mflag, nflag, pflag, qflag, rflag, sflag, tflag;
int uflag, xflag, flag1;
int maxlen[LastFormat];
int out, rval, sort, textwidth;
FileList dirl, fl;
int (*cmp[LastSort]) (const void *, const void *) = {
  [AlnumSort] = alnumsort, [NoSort] = nosort, [SizeSort] = sizesort, [TimeSort] = timesort };
void (*put[LastPut]) (FileList *) = {
  [ColumnPut] = columnput, [ListPut] = listput, [LongPut] = longput, [StreamPut] = streamput };

void
addfile(FileList *l, File *f) {
  if(l->cap == l->len) {
    l->cap += LINCR;
    l->f = realloc(l->f, sizeof(File)*l->cap);
  }
  l->f[l->len++] = *f;
}

int
alnumsort(const void *v1, const void *v2) {
  const File *f1, *f2;
  int r;

  f1 = (const File *)v1;
  f2 = (const File *)v2;
  r = strcoll(f1->name, f2->name);
  return(rflag ? -r : r);
}

void
columnput(FileList *l) {
  int h, i, j, k, n, len, w;

  len = 0;
  for(i = 0; i < l->len; i++) {
    n = strlen(filename(&l->f[i]));
    if(len < n)
      len = n+1;
  }
  w = textwidth / len;
  h = (l->len + (w - 1)) / w;
  for(i = 0; i < h; i++) {
    k = xflag ? i*w : i;
    for(j = 0; j < w && k < l->len; j++) {
      printf("%*s", -len, filename(&l->f[i]));
      k += xflag ? 1 : h;
    }
    printf("\n");
  }
}

char *
date(struct timespec *ts) {
  static char s[13];
  time_t tdiff;

  tdiff = time(nil) - ts->tv_sec;
  strftime(s, sizeof s, (tdiff >= 0 && tdiff < 15552000) ? "%b %e %H:%M" : "%b %e  %Y", localtime(&ts->tv_sec));
  return s;
}

char *
filename(File *f) {
  static char buf[NAME_MAX];

  snprintf(buf, NAME_MAX, "%s%s%c", prefix(&f->info), f->name, mark(f->info.st_mode));
  return buf;
}

void
initlist(FileList *l) {
  l->cap = LINCR;
  l->len = 0;
  l->f = malloc(sizeof(File)*l->cap);
}

void
listput(FileList *l) {
  int i;

  for(i = 0; i < l->len; i++)
    printf("%s\n", filename(&l->f[i]));
}

void
ls(char *s) {
  static File f;
  struct stat sb;

  if(((Fflag+dflag+lflag)
    && !(Hflag+Lflag)) ? lstat(s, &sb) : stat(s, &sb)) {
    alert("%s: %m", s);
    rval = 1;
    return;
  }
  f.name = s;
  f.info = sb;
  if(dflag || !S_ISDIR(f.info.st_mode)) {
    addfile(&fl, &f);
    return;
  }
  if(Rflag) {
    if(nftw(s, walk, 20, Lflag ? 0 : FTW_PHYS) < 0) {
      alert("%s: %m", s);
      rval = 1;
    }
  } else {
    addfile(&dirl, &f);
  }
}

char
mark(mode_t m) {
  if(!(Fflag+pflag))
    return '\0';
  if(S_ISDIR(m))
    return '/';
  if(pflag)
    return '\0';
  if(S_ISFIFO(m))
    return '|';
  if(S_ISLNK(m))
    return '@';
  if(m & (S_IXUSR|S_IXGRP|S_IXOTH))
    return '*';
  return '\0';
}

char *
mode(mode_t m) {
  static char s[11];

  memset(s, '-', sizeof s);
  s[10] = '\0';
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
    else s[3] = 'S';
  } else if(m & S_IXUSR)
    s[3] = 'x';
  if(m & S_IRGRP)
    s[4] = 'r';
  if(m & S_IWGRP)
    s[5] = 'w';
  if(m & S_ISGID) {
    if(m & S_IXGRP)
      s[6] = 's';
    else s[6] = 'S';
  } else if(m & S_IXGRP)
    s[6] = 'x';
  if(m & S_IROTH)
    s[7] = 'r';
  if(m & S_IWOTH)
    s[8] = 'w';
  if(m & S_IXOTH)
    s[9] = 'x';
  return s;
}

int
nosort(const void *v1, const void *v2) {
  return 0;
}

void
output(FileList *l) {
  int i;
  char *p;

  if(l->len == 0)
    return;
  if(isatty(1) || qflag)
    for(i = 0; i < l->len; i++)
      for(p = l->f[i].name; *p; p++)
        if(!isprint((int)*p) || *p == '\t')
          *p = '?';
  setformat(l);
  put[out](l);
}

char *
prefix(struct stat *sb) {
  int n;
  static char s[44];

  n = 0;
  memset(s, 0, sizeof s);
  if(iflag)
    n = snprintf(s, sizeof s, "%*lld ", maxlen[SerialFormat], sb->st_ino);
  if(sflag)
    snprintf(s+n, sizeof s - n, "%*lld ", maxlen[BlockFormat], sb->st_blocks);
  return s;
}

void
longput(FileList *l) {
}

void
setformat(FileList *l) {
  int i, n;
  char s[21];

  memset(&maxlen, 0, sizeof(int)*LastFormat);
  if(mflag || !(lflag+iflag+sflag))
    return;
  for(i = 0; i < l->len; i++) {
    if(iflag && maxlen[SerialFormat] < (n = snprintf(s, sizeof s, "%lld", l->f[i].info.st_ino)))
      maxlen[SerialFormat] = n;
    if(sflag && maxlen[BlockFormat] < (n = snprintf(s, sizeof s, "%lld", l->f[i].info.st_blocks)))
      maxlen[BlockFormat] = n;
    if(!lflag)
      continue;
    //lflag max lengths }
}

void
settextwidth(void) {
  struct winsize w;

  if(isatty(1) && ioctl(1, TIOCGWINSZ, &w) != -1)
    textwidth = w.ws_col;
  else textwidth = TEXTWIDTH;
}

int
sizesort(const void *v1, const void *v2) {
  const File *f1, *f2;
  off_t odiff;
  int r;

  f1 = (const File *)v1;
  f2 = (const File *)v2;
  r = (odiff = f1->info.st_size - f2->info.st_size) ? odiff : strcoll(f1->name, f2->name);
  return(rflag ? -r : r);
}

void
sortlist(FileList *l) {
  qsort(l->f, l->len, sizeof(File), cmp[sort]);
}

void
streamput(FileList *l) {
  int i, n;
  char *line;

  n = 0;
  for(i = 0; i < l->len; i++) {
    line = filename(&l->f[k]);
    n += strlen(line);
    if(i > 0) {
      if(n < textwidth-1) {
        printf(", ");
        n += 2;
      } else {
        printf(",\n");
        n = strlen(line);
      }
    }
    println(line);
  }
  print('\n');
}

int
timesort(const void *v1, const void *v2) {
  const File *f1, *f2;
  time_t tdiff, t1, t2;
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
  return(rflag ? -r : r);
}

int
walk(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
  File f;

  if(tflag == FTW_D) {
    f.name = strdup(fpath);
    f.info = *sb;
    addfile(&dirl, &f);
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  DIR *d;
  struct dirent *dent;
  File f;
  FileList l;
  int i, j;
  char path[PATH_MAX];
  struct stat sb;
  blkcnt_t total;

  Aflag = Cflag = Fflag = Hflag = Lflag = Rflag = Sflag = aflag = cflag = dflag = fflag = iflag = kflag = lflag = mflag = nflag = pflag = qflag = rflag = sflag = tflag = uflag = xflag = 0;
  rval = 0;
  out = ListPut;
  sort = AlnumSort;
  ARGBEGIN("ACFHLRSacdfiklmnpqrstux1"){
  case 'A':
    Aflag = 1;
    break;
  case 'C':
    out = ColumnPut;
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
    sort = SizeSort;
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
    if(out == LongPut)
      out = ListPut;
    sort = NoSort;
    break;
  case 'i':
    iflag = 1;
    break;
  case 'k':
    break;
  case 'l':
    nflag = 0;
    if(sort == NoSort)
      sort = AlnumSort;
    out = LongPut;
    break;
  case 'm':
    out = StreamPut;
    break;
  case 'n':
    nflag = 1;
    out = LongPut;
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
    sort = TimeSort;
    break;
  case 'u':
    cflag = 0;
    uflag = 1;
    break;
  case 'x':
    out = ColumnPut;
    xflag = 1;
    break;
  case '1':
    out = ListPut;
    break;
  default:
    fprint("usage: ls [-ACFRSacdfiklmnpqrstux1] [-H|-L] [file...]\n", stderr);
    exit(1);
  }ARGEND 
  settextwidth();
  initlist(&dirl);
  initlist(&fl);
  if(argc == 0)
    ls(".");
  for(i = 0; i < argc; i++)
      ls(argv[i]);
  sortlist(&fl);
  output(&fl);
  sortlist(&dirl);
  initlist(&l);
  for(i = 0; i < dirl.len; i++) {
    if(!(d = opendir(dirl.f[i].name))) {
      alert("can't open %s: %m", dirl.f[i].name);
      rval = 1;
      continue;
    }
    while((dent = readdir(d))) {
      if((!aflag && dent->d_name[0] == '.')
        || (Aflag && (!strcmp(dent->d_name, ".")
        || !strcmp(dent->d_name, ".."))))
        continue;
      snprintf(path, PATH_MAX, "%s/%s", dirl.f[i].name, dent->d_name);
      if(((Fflag+dflag+lflag)
        && !Lflag) ? lstat(path, &sb) : stat(path, &sb)) {
        alert("%s: %m", path);
        rval = 1;
        continue;
      }
      f.name = strdup(dent->d_name);
      f.info = sb;
      addfile(&l, &f);
    }
    closedir(d);
    sortlist(&l);
    if(dirl.len+fl.len > 1)
      printf("\n%s:\n", dirl.f[i].name);
    if(lflag+nflag+sflag) {
      total = 0;
      for(j = 0; j < l.len; j++)
        total += l.f[j].info.st_blocks;
      printf("total %lld\n", total);
    }
    output(&l);
    for(j = 0; j < l.len; j++)
      free(l.f[j].name);
    l.len = 0;
  }
  exit(rval);
}
