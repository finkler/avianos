#include <u.h>
#include <avian.h>
#include <sys/statvfs.h>

#define PATH_MOUNTS "/proc/mounts"

typedef struct FSInfo FSInfo;
struct FSInfo {
  char name[NAME_MAX];
  char path[PATH_MAX];
  struct statvfs sb;
  FSInfo *next;
};

long blksiz;
FSInfo *fsinf;

void
output(FSInfo *f) {
  ulong avail, blocks;
  ulong cap, used;

  blocks = f->sb.f_blocks * f->sb.f_frsize;
  used = (f->sb.f_blocks - f->sb.f_bfree) * f->sb.f_frsize;
  avail = f->sb.f_bavail * f->sb.f_frsize;
  cap = (used * 100 + (blocks - 1)) / blocks;
  printf("%-12s %12lu %12lu %12lu %8lu%%  %s\n",
    f->name, blocks/blksiz, used/blksiz, avail/blksiz, cap, f->path);
}

void
parsemounts(void) {
  char *buf;
  FILE *f;
  FSInfo *tm, *r;

  f = fopen(PATH_MOUNTS, "r");
  if(f == nil)
    fatal(1, "can't open %s: %m", PATH_MOUNTS);
  while((buf = fgetln(f))) {
    tm = malloc(sizeof(FSInfo));
    sscanf(buf, "%s %s", tm->name, tm->path);
    if(statvfs(tm->path, &tm->sb) || tm->sb.f_blocks == 0) {
      free(tm);
      continue;
    }
    tm->next = nil;
    if(fsinf == nil) {
      fsinf = tm;
    } else {
      r = fsinf;
      while(r->next)
        r = r->next;
      r->next = tm;
    }
  }
  fclose(f);
}

int
main(int argc, char *argv[]) {
  int i;
  struct statvfs sb;
  FSInfo *r;

  blksiz = 512;
  fsinf = nil;
  ARGBEGIN("kP"){
  case 'k':
    blksiz = 1024;
    break;
  case 'P':
    break;
  default:
    fprint(stderr, "usage: df [-kP] [file...]\n");
    exit(1);
  }ARGEND 
  
  parsemounts();
  printf("%-12s %5ld-blocks %12s %12s  Capacity  Mounted on\n",
    "Filesystem", blksiz, "Used", "Available");
  if(argc == 0)
    for(r = fsinf; r; r = r->next)
      output(r);
  for(i = 0; i < argc; i++) {
    if(statvfs(argv[i], &sb)) {
      alert("%s: %m", argv[i]);
      continue;
    }
    for(r = fsinf; r; r = r->next)
      if(r->sb.f_fsid == sb.f_fsid) {
        output(r);
        break;
      }
  }
  exit(rval);
}
