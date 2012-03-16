#include <u.h>
#include <avian.h>
#include <linux/keyboard.h>
#include <sys/ioctl.h>
#include <sys/kd.h>

#define CONS    "/dev/console"
#define STR_LEN 512

int fd;
char functab[MAX_NR_FUNC][STR_LEN];
ushort keytab[MAX_NR_KEYMAPS][NR_KEYS];

char *
escapestr(char *s) {
  char buf[STR_LEN+1], *p;
  int i;

  i = 0;
  for(p = s; *p && i < STR_LEN; p++) {
    if(*p == '\\')
      buf[i++] = (char)strtol(++p, &p, 8);
    if(*p)
      buf[i++] = *p;
  }
  buf[i] = '\0';
  strcpy(s, buf);
  return s;
}

void
setdiacs(void) {
  static struct kbdiacrs kd;
  
  if(ioctl(fd, KDSKBDIACR, &kd))
    fatal(1, "ioctl: %m");
}

void
setfuncs(void) {
  int i;
  struct kbsentry ks;

  for(i = 0; i < MAX_NR_FUNC; i++) {
    ks.kb_func = i;
    if(functab[i][0])
      strncpy((char *)ks.kb_string, functab[i], sizeof ks.kb_string);
    else
      memset(ks.kb_string, 0, sizeof ks.kb_string);
    if(ioctl(fd, KDSKBSENT, &ks))
      fatal(1, "ioctl: %m");
  }
}

void
setkeys(void) {
  int i, j;
  struct kbentry ke;

  for(i = 0; i < MAX_NR_KEYMAPS; i++) {
    ke.kb_table = i;
    if(keytab[i][0]) {
      for(j = 0; j < NR_KEYS; j++) {
        ke.kb_index = j;
        ke.kb_value = keytab[i][j];
        if(ioctl(fd, KDSKBENT, &ke))
          fatal(1, "ioctl: %m");
      }
    } else {
      ke.kb_index = 0;
      ke.kb_value = K_NOSUCHMAP;
      if(ioctl(fd, KDSKBENT, &ke))
        fatal(1, "ioctl: %m");
    }
  }
}

void
usage(void) {
  fprint(stderr, "usage: keymap file\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *buf, *e;
  FILE *f;
  int i, j, n;
  int mod, st;
  ushort val;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND
  
  if(argc != 1)
    usage();
  i = j = mod = st = 0;
  f = fopen(argv[0], "r");
  if(f == nil)
    fatal(1, "can't open %s: %m", argv[0]);
  while((buf = fgetln(f))) {
    if(!strncmp(buf, "COMMENT", 7))
      continue;
    if(!strcmp(buf, "END")) {
      if(!st)
        fatal(1, "syntax error: missing BEGIN");
      st = 0;
      if(i > 0)
        while(i < NR_KEYS)
          keytab[mod][i++] = K_HOLE;
      continue;
    }
    if(!strcmp(buf, "BEGIN")) {
      i = 0;
      if(st)
        fatal(1, "syntax error: missing END");
      st = 1;
      continue;
    }
    if(st == 1) {
      if(strncmp(buf, "MODIFIER ", 9))
        fatal(1, "syntax error: missing MODIFIER");
      mod = strtol(buf+9, &e, 10);
      if(*e != 0 || mod >= MAX_NR_KEYMAPS)
        fatal(1, "syntax error: invalid modifier %s", buf+9);
      st++;
      continue;
    }
    if(st) {
      if(!strcmp(buf, "UNDEF")) {
        val = K_HOLE;
      } else if(!strncmp(buf, "STRING ", 7)) {
        if(j >= MAX_NR_FUNC)
          fatal(1, "syntax error: too many strings");
        val = ((ushort)j) | 0xf100;
        strncpy(functab[j++], escapestr(buf+7), STR_LEN);
      } else {
        val = (ushort)strtol(buf, &e, 16);
        if(*e != 0)
          fatal(1, "syntax error: invalid keycode %s", buf);
      }
      if(i >= NR_KEYS)
        fatal(1, "syntax error: too many keycodes");
      keytab[mod][i++] = val;
      continue;
    }
    fatal(1, "syntax error: invalid keyword %s", buf);
  }
  if(ferror(f))
    fatal(1, "error reading %s: %m", argv[0]);
  fclose(f);
  if(st)
    fatal(1, "syntax error: missing END");
  fd = open(CONS, O_RDONLY);
  if(fd < 0)
    fatal(1, "can't open %s: %m", CONS);
  setdiacs();
  setfuncs();
  setkeys();
}
