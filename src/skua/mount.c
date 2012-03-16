#include <u.h>
#include <avian.h>
#include <sys/mount.h>

typedef struct {
  char *s;
  int v;
} Map;

char *data, *type;
int flags;
Map m[] = {
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

void
parseopts(char *s) {
  char *key, *p;
  Map *r;

  data = malloc(strlen(s)+1);
  *data = '\0';
  p = data;
  key = strtok(s, ",");
  for(;;) {
    for(r = m; r->s; r++)
      if(!strcmp(r->s, key)) {
        flags |= r->v;
        break;
      }
    if(r->s != nil) {
      if(*data != '\0')
        *p++ = ',';
      strcpy(p, key);
      p += strlen(p);
    }
    key = strtok(nil, ",");
    if(key == nil)
      break;
  }
  if(*data == '\0') {
    free(data);
    data = nil;
  }
}

void
usage(void) {
  fprint(stderr, "usage: mount [-o options] -t type device dir\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
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
  if(mount(argv[0], argv[1], type, flags, data))
    fatal(1, "can't mount %s on %s: %m", argv[0], argv[1]);
}
