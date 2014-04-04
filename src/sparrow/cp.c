#include <u.h>
#include <avian.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>

int  ask(char*);
void copydir(char*, char*);
void copy(int, char*, int, char*);
void cp(char*, char*, int);

int fflag;
int iflag;
int pflag;
int Rflag;
int (*stat_r[2])(const char*, struct stat*);

int
ask(char *s)
{
  char *ans;

  fprintf(stderr, "cp: overwrite %s?[n]: ", s);
  ans = fgetln(stdin);
  if(*ans != 'y' && *ans != 'Y')
    return 0;
  return 1;
}

void
copydir(char *src, char *dest)
{
  DIR *d;
  struct dirent *ent;
  char *p1, *p2;

  d = opendir(src);
  if(d == nil){
    alert("opendir %s: %m", src);
    return;
  }
  while((ent = readdir(d))){
    if(!strcmp(ent->d_name, ".")
    || !strcmp(ent->d_name, ".."))
      continue;
    p1 = stradd(src, "/", ent->d_name);
    p2 = stradd(dest, "/", ent->d_name);
    cp(p1, p2, 0);
    free(p2);
    free(p1);
  }
  closedir(d);
}

void
copy(int ifd, char *src, int ofd, char *dest)
{
  char buf[8192];
  int n;

  while((n = read(ifd, buf, sizeof buf)) > 0)
    if(write(ofd, buf, n) != n){
      alert("write %s: %m", dest);
      return;
    }
  if(n == -1)
    alert("read %s: %m", src);
}

void
cp(char *src, char *dest, int op)
{
  char buf[PATH_MAX+1];
  int flag, ifd, isthere, n, ofd;
  struct stat isb, osb;
  struct timespec times[2];

  if(stat_r[op](src, &isb)){
    alert("stat %s: %m", src);
    return;
  }
  isthere = 1;
  if(stat_r[op](dest, &osb)){
    if(errno != ENOENT){
      alert("stat %s: %m", dest);
      return;
    }
    isthere = 0;
  }else if(isb.st_dev == osb.st_dev
  && isb.st_ino == osb.st_ino){
    alert("%s is %s", src, dest);
    return;
  }
  flag = 0;
  switch(isb.st_mode&S_IFMT){
  case S_IFDIR:
    if(!Rflag){
      alert("%s: is a directory", src);
      return;
    }
    if(isthere){
      if(!S_ISDIR(osb.st_mode)){
        alert("%s: not a directory", dest);
        return;
      }
    }else if(mkdir(dest, isb.st_mode|S_IRWXU)){
      alert("mkdir %s: %m", dest);
      return;
    }
    copydir(src, dest);
    break;
  case S_IFREG:
    if(isthere){
      if(iflag && ask(dest))
        return;
      ofd = open(dest, O_WRONLY|O_TRUNC);
      if(ofd != -1)
        goto Opened;
      if(!fflag || unlink(dest))
        return;
    }
    ofd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, isb.st_mode);
  Opened:
    ifd = open(src, O_RDONLY);
    if(ifd < 0 || ofd < 0){
      alert("open %s: %m", ifd<0 ? src : dest);
      return;
    }
    copy(ifd, src, ofd, dest);
    close(ifd);
    close(ofd);
    break;
  case S_IFBLK:
  case S_IFCHR:
    if(Rflag){
      if(mknod(dest, isb.st_mode, isb.st_dev)){
        alert("mknod %s: %m", dest);
        return;
      }
      break;
    }
  case S_IFIFO:
    if(Rflag){
      if(mkfifo(dest, isb.st_mode)){
        alert("mkfifo %s: %m", dest);
        return;
      }
      break;
    }
  case S_IFLNK:
    flag = AT_SYMLINK_NOFOLLOW;
    if(Rflag){
      n = readlink(src, buf, PATH_MAX);
      if(n < 0){
        alert("readlink %s: %m", src);
        return;
      }
      buf[n] = '\0';
      if(symlink(buf, dest)){
        alert("symlink %s: %m", dest);
        return;
      }
      break;
    }
  default:
    alert("%s: invalid file type", src);
    return;
  }
  if(pflag){
    times[0] = isb.st_atim;
    times[1] = isb.st_mtim;
    if(utimensat(AT_FDCWD, dest, times, flag))
      alert("utimensat %s: %m", dest);
    chown(dest, isb.st_uid, isb.st_gid);
    if(chmod(dest, isb.st_mode))
      alert("chmod %s: %m", dest);
  }
}

void
usage(void)
{
  fprint(stderr, "usage: cp [-Pfip] source_file target_file\n"
    "       cp [-Pfip] source_file... target\n"
    "       cp -R [-H|-L|-P] [-fip] source_file... target\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int i;
  char *p;
  struct stat sb;

  stat_r[0] = stat_r[1] = stat;
  ARGBEGIN("fHiLPpR"){
  case 'f':
    fflag = 1;
    break;
  case 'H':
    stat_r[0] = stat;
    stat_r[1] = lstat;
    break;
  case 'i':
    iflag = 1;
    break;
  case 'L':
    stat_r[0] = lstat;
    stat_r[1] = stat;
    break;
  case 'P':
    stat_r[0] = stat_r[1] = lstat;
    break;
  case 'p':
    pflag = 1;
    break;
  case 'R':
    Rflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc < 2)
    usage();
  if(!stat_r[0](argv[argc-1], &sb) && S_ISDIR(sb.st_mode))
    for(i = 0; i < argc-1; i++){
      p = stradd(argv[argc-1], "/", basename(cleanname(argv[i])));
      cp(argv[i], p, 1);
      free(p);
    }
  else if(argc == 2)
    cp(argv[0], argv[1], 1);
  else
    fatal(1, "%s: not a directory", argv[argc-1]);
  exit(rval);
}
