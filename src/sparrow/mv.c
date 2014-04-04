#include <u.h>
#include <avian.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>

#define S_WRACC(m) (m&(S_IWUSR|S_IWGRP|S_IWOTH))

int fflag;
int iflag;

int
ask(char *s)
{
  char *ans;

  fprintf(stderr, "mv: overwrite %s?[n]: ", s);
  ans = fgetln(stdin);
  if(*ans != 'y' && *ans != 'Y')
    return 0;
  return 1;
}

void
cp(char *src, char *dst)
{
  char cmd[10], *p;

  strcpy(cmd, "cp -RPp");
  if(fflag)
    strcat(cmd, "f");
  if(iflag)
    strcat(cmd, "i");
  p = stradd(cmd, " ", src, " ", dst);
  system(p);
  free(p);
  p = stradd("rm -fr ", src);
  system(p);
  free(p);
}

void
mv(char *old, char *new)
{
  int isthere;
  struct stat nsb, osb;

  if(stat(old, &osb)){
    alert("stat %s: %m", old);
    return;
  }
  isthere = 1;
  if(stat(new, &nsb)){
    if(errno != ENOENT){
      alert("stat %s: %m", new);
      return;
    }
    isthere = 0;
  }else if(nsb.st_dev == osb.st_dev
  && nsb.st_ino == osb.st_ino){
    if(strcmp(basename(cleanname(old)), basename(cleanname(new)))){
      if(unlink(new)){
        alert("unlink %s: %m", new);
        return;
      }
      isthere = 0;
    }else
      return;
  }
  if(isthere && !fflag
  && ((!S_WRACC(nsb.st_mode) && isatty(0)) || iflag)
  && ask(new))
    return;
  if(rename(old, new)){
    alert("rename %s: %m", new);
    if(errno == EXDEV)
      cp(old, new);
  }
}

void
usage(void)
{
  fprint(stderr, "usage: mv [-if] source_file target_file\n"
         "       mv [-if] source_file... target_dir\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int i;
  char *p;
  struct stat sb;

  fflag = iflag = 0;
  ARGBEGIN("fi"){
  case 'f':
    fflag = 1;
    iflag = 0;
    break;
  case 'i':
    fflag = 0;
    iflag = 1;
    break;
  default:
    usage();
  }ARGEND

  if(argc < 2)
    usage();
  if(!stat(argv[argc-1], &sb) && S_ISDIR(sb.st_mode))
    for(i = 0; i < argc-1; i++){
      p = stradd(argv[argc-1], "/", basename(cleanname(argv[i])));
      mv(argv[i], p);
      free(p);
    }
  else if(argc == 2)
    mv(argv[0], argv[1]);
  else
    fatal(1, "%s: not a directory", argv[argc-1]);
  exit(rval);
}
