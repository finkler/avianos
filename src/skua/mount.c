#include <u.h>
#include <avian.h>
#include <sys/mount.h>

#define PRINTK "/proc/sys/kernel/printk"

typedef struct Map Map;
struct Map
{
  char *s;
  int  v;
};

char *data;
char *type;
int  flags;
Map  m[] = {
 {"bind",        MS_BIND},
 {"defaults",    0},
 {"dirsync",     MS_DIRSYNC},
 {"mandlock",    MS_MANDLOCK},
 {"move",        MS_MOVE},
 {"noatime",     MS_NOATIME},
 {"nodev",       MS_NODEV},
 {"nodiratime",  MS_NODIRATIME},
 {"noexec",      MS_NOEXEC},
 {"nosuid",      MS_NOSUID},
 {"ro",          MS_RDONLY},
 {"relatime",    MS_RELATIME},
 {"remount",     MS_REMOUNT},
 {"silent",      MS_SILENT},
 {"strictatime", MS_STRICTATIME},
 {"sync",        MS_SYNCHRONOUS},
 {nil, 0}
};
char *supp[] = {
  "ext4",    "vfat",
  "iso9660", "udf",
  nil
};

void
parseopts(char *s)
{
  char *key, *p;
  Map *r;

  data = malloc(strlen(s)+1);
  *data = '\0';
  p = data;
  key = strtok(s, ",");
  for(;;){
    for(r = m; r->s; r++)
      if(!strcmp(r->s, key)){
        flags |= r->v;
        break;
      }
    if(r->s != nil){
      if(*data != '\0')
        *p++ = ',';
      strcpy(p, key);
      p += strlen(p);
    }
    key = strtok(nil, ",");
    if(key == nil)
      break;
  }
  if(*data == '\0'){
    free(data);
    data = nil;
  }
}

int
printk(int toggle)
{
  static char level[16];
  FILE *f;

  if(toggle < 0)
    return -1;
  f = fopen(PRINTK, "rw");
  if(f == nil)
    return -1;
  if(toggle == 0){
    if(fgets(level, 16, f) == nil)
      return -1;
    fprint(f, "2 2 2 2\n");
  }else
    fprint(f, level);
  fclose(f);
  return 1;
}

void
usage(void)
{
  fprint(stderr, "usage: mount [-o options] [-t type] "
         "device mount_point\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  char **p;
  int set;

  data = type = nil;
  flags = 0;
  ARGBEGIN("o:t:"){
  case 'o':
    parseopts(optarg);
    break;
  case 't':
    type = optarg;
    break;
  default:
    usage();
  }ARGEND

  if(argc != 2)
    usage();
  if(type == nil){
    set = printk(0);
    for(p = supp; *p; p++)
      if(!mount(argv[0], argv[1], *p, flags, data))
        break;
    printk(set);
    if(*p == nil)
      fatal(1, "%m");
  }else if(mount(argv[0], argv[1], type, flags, data))
    fatal(1, "%m");
}
