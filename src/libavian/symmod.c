#include <u.h>
#include <avian.h>
#include <sys/stat.h>

#define S_ISRWXU (S_IRWXU|S_ISUID)
#define S_ISRWXG (S_IRWXG|S_ISGID)
#define S_ISRWXA (S_ISRWXU|S_ISRWXG|S_IRWXO)
#define S_IRALL  (S_IRUSR|S_IRGRP|S_IROTH)
#define S_IWALL  (S_IWUSR|S_IWGRP|S_IWOTH)
#define S_IXALL  (S_IXUSR|S_IXGRP|S_IXOTH)

#define IS_WHO(c)     ((c)=='u'||(c)=='g'||(c)=='o'||(c)=='a')
#define IS_PERMCPY(c) ((c)=='u'||(c)=='g'||(c)=='o')
#define IS_PERM(c)    ((c)=='r'||(c)=='w'||(c)=='x'||(c)=='s')
#define IS_OP(c)      ((c)=='+'||(c)=='-'||(c)=='=')

static void action(int, char *);
static void parse(char *s);

static uint mod, who;

void
action(int op, char *perm) {
  uint cpy, set;
  char *p;
  
  p = perm;
  if(op == '=') {
    if(*p == '\0')
      mod ^= mod&(who&(S_ISRWXU|S_ISRWXG|S_IRWXO));
    cpy = 0;
    while(IS_PERMCPY(*p)) {
      switch(*p++) {
      case 'u':
        cpy = (mod&S_ISRWXU)>>6;
        break;
      case 'g':
        cpy = (mod&S_ISRWXG)>>3;
        break;
      case 'o':
        cpy = mod&S_IRWXO;
        break;
      }
      if(who&S_ISRWXU) {
        mod &= 0077;
        mod |= cpy<<6;
      }
      if(who&S_ISRWXG) {
        mod &= 0707;
        mod |= cpy<<3;
      }
      if(who&S_IRWXO) {
        mod &= 0770;
        mod |= cpy;
      }
    }
    return;
  }
  for(set = 0; IS_PERM(*p); p++)
    switch(*p) {
    case 'r':
      set |= S_IRALL;
      break;
    case 'w':
      set |= S_IWALL;
      break;
    case 'x':
      set |= S_IXALL;
      break;
    }
  if(op == '+')
    mod |= set&who;
  else if(op == '-')
    mod ^= set&(who&mod);
}

void
parse(char *s) {
  char *e, *p, *perm;
  int op;
  
  who = 0;
  for(p = s; IS_WHO(*p); p++)
    switch(*p) {
    case 'a':
      who = S_ISRWXA;
      break;
    case 'g':
      who |= S_ISRWXG;
      break;
    case 'o':
      who |= S_IRWXO;
      break;
    case 'u':
      who |= S_ISRWXU;
      break;
    }
  if(who == 0)
    who = S_ISRWXA;
  for(;;) {
    op = *p++;
    if(op == 0)
      break;
    perm = strdup(p);
    e = strpbrk(perm, "+-=");
    if(e != nil)
      *e = '\0';
    action(op, perm);
    free(perm);
    while(*p && !IS_OP(*p))
      p++;
  }
}

uint
symmod(char *str, uint m) {
  char *p, *s;
  
  mod = m;
  p = str;
  while((p = strchr(p, 'X')))
    if(mod&S_IXALL)
      *p = 'x';
  s = strdup(str);
  for(p = strtok(s, ","); p; p = strtok(nil, ","))
    parse(p);
  free(s);
  return mod;
}
