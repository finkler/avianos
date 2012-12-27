#include <u.h>
#include <avian.h>
#include <termios.h>

#ifndef IUTF8
#define IUTF8 0040000
#endif 
#define CTRL(c)   ((c)=='?'?0x7f:(c)&0x1f)
#define UNCTRL(c) ((c)==0x7f?'?':(c)+0x40)

typedef struct {
  char *key;
  uint val;
  uint def;
} Map;

uint getbaudval(char *);
char *getbaudstr(uint);
void putcchars(void);
void putflags(Map *, uint *);
void putline(char *, ...);
void putsaved(void);
void putsize(void);
void putspeed(void);
void readsaved(char *);
int setcchars(char *, char *);
int setcomb(char *);
int setflags(char *, Map *, uint *);
int setsize(char *);
int setspeed(char *, char *);

int aflag, gflag;
int rval;
struct termios tc;
Map baud[] = {
  {"0",     B0,     0},
  {"50",    B50,    0},
  {"75",    B75,    0},
  {"110",   B110,   0},
  {"134",   B134,   0},
  {"150",   B150,   0},
  {"200",   B200,   0},
  {"300",   B300,   0},
  {"600",   B600,   0},
  {"1200",  B1200,  0},
  {"1800",  B1800,  0},
  {"2400",  B2400,  0},
  {"4800",  B4800,  0},
  {"9600",  B9600,  0},
  {"19200", B19200, 0},
  {"38400", B38400, 0},
  {nil, 0, 0 }
};
Map cchar[] = {
  {"eof",   VEOF,   CTRL('D')      },
  {"eol",   VEOL,   _POSIX_VDISABLE},
  {"erase", VERASE, CTRL('?')      },
  {"intr",  VINTR,  CTRL('C')      },
  {"kill",  VKILL,  CTRL('U')      },
  {"min",   VMIN,   1              },
  {"quit",  VQUIT,  CTRL('\\')     },
  {"start", VSTART, CTRL('Q')      },
  {"stop",  VSTOP,  CTRL('S')      },
  {"susp",  VSUSP,  CTRL('Z')      },
  {"time",  VTIME,  0              },
  {nil, 0, 0}
};
Map control[] = {
  {"clocal", CLOCAL, 0    },
  {"cread",  CREAD,  CREAD},
  {"cstopb", CSTOPB, 0    },
  {"hupcl",  HUPCL,  HUPCL},
  {"hup",    HUPCL,  HUPCL},
  {"parenb", PARENB, 0    },
  {"parodd", PARODD, 0    },
  {nil, 0, 0}
};
Map csize[] = {
  {"cs5", CS5, 0},
  {"cs6", CS6, 0},
  {"cs7", CS7, 0},
  {"cs8", CS8, 0},
  {nil, 0, 0}
};
Map input[] = {
  {"brkint", BRKINT, BRKINT},
  {"icrnl",  ICRNL,  ICRNL },
  {"ignbrk", IGNBRK, 0     },
  {"igncr",  IGNCR,  0     },
  {"ignpar", IGNPAR, 0     },
  {"inlcr",  INLCR,  0     },
  {"inpck",  INPCK,  0     },
  {"istrip", ISTRIP, 0     },
  {"ixany",  IXANY,  0,    },
  {"ixoff",  IXOFF,  0     },
  {"ixon",   IXON,   IXON  },
  {"parmrk", PARMRK, 0     },
  {nil, 0, 0}
};
Map local[] = {
  {"echo",   ECHO,   ECHO  },
  {"echoe",  ECHOE,  ECHOE },
  {"echok",  ECHOK,  ECHOK },
  {"echonl", ECHONL, 0     },
  {"icanon", ICANON, ICANON},
  {"iexten", IEXTEN, IEXTEN},
  {"isig",   ISIG,   ISIG  },
  {"noflsh", NOFLSH, 0     },
  {"tostop", TOSTOP, 0     },
  {nil, 0, 0}
};
Map output[] = {
  {"opost", OPOST, OPOST},
  {nil, 0, 0}
};

uint
getbaudval(char *s) {
  Map *m;

  for(m = baud; m->key; m++)
    if(!strcmp(m->key, s))
      return m->val;
  return (uint)-1;
}

char *
getbaudstr(uint v) {
  Map *m;

  for(m = baud; m->key; m++)
    if(m->val == v)
      return m->key;
  return nil;
}

void
putcchars(void) {
  uchar cc;
  Map *m;
  char s[4];

  for(m = cchar; m->key; m++) {
    cc = tc.c_cc[m->val];
    if(aflag || cc != m->def) {
      if(!strcmp(m->key, "min") || !strcmp(m->key, "time"))
        sprintf(s, "%u", cc);
      else if(cc == _POSIX_VDISABLE)
        strcpy(s, "^-");
      else if(isprint(cc))
        sprintf(s, "%c", cc);
      else
        sprintf(s, "^%c", UNCTRL(cc));
      putline("%s = %s;", m->key, s);
    }
  }
}

void
putflags(Map *m, uint *flag) {
  for(; m->key; m++)
    if(aflag || (m->def != (*flag & m->val))) {
      if(*flag & m->val)
        putline("%s", m->key);
      else
        putline("-%s", m->key);
    }
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
putsaved(void) {
  Map *m;

  printf("%x:%x:%x:%x", tc.c_cflag, tc.c_iflag,
    tc.c_lflag, tc.c_oflag);
  for(m = cchar; m->key; m++)
    printf(":%x", tc.c_cc[m->val]);
  printf(":%x:%x\n", cfgetispeed(&tc), cfgetospeed(&tc));
}

void
putsize(void) {
  Map *m;

  for(m = csize; m->key; m++)
    if((aflag || m->val != CS8) && ((tc.c_cflag&CSIZE) == m->val)) {
      putline("%s", m->key);
      break;
    }
}

void
putspeed(void) {
  uint ispeed, ospeed;

  ispeed = cfgetispeed(&tc);
  ospeed = cfgetospeed(&tc);
  if(ispeed == ospeed)
    printf("speed %s baud;\n", getbaudstr(ispeed));
  else
    printf("ispeed %s baud; ospeed %s baud;\n",
      getbaudstr(ispeed), getbaudstr(ospeed));
}

void
readsaved(char *s) {
  uint *flags[4], speed[2];
  int i;
  Map *m;
  char *p;

  memset(&tc, 0, sizeof tc);
  flags[0] = &tc.c_cflag;
  flags[1] = &tc.c_iflag;
  flags[2] = &tc.c_lflag;
  flags[3] = &tc.c_oflag;
  p = s;
  for(i = 0; i < 4; i++) {
    *flags[i] = (uint)strtol(p, &p, 16);
    if(*p == ':')
      p++;
    else
      fatal(1, "invalid operand %s", s);
  }
  for(m = cchar; m->key; m++) {
    tc.c_cc[m->val] = (uchar)strtol(p, &p, 16);
    if(*p == ':')
      p++;
    else
      fatal(1, "invalid operand %s", s);
  }
  for(i = 0; i < 2; i++) {
    speed[i] = strtol(p, &p, 16);
    if(*p == ':')
      p++;
    else if(i && *p == '\0')
      break;
    else
      fatal(1, "invalid operand %s", s);
  }
  cfsetispeed(&tc, speed[0]);
  cfsetospeed(&tc, speed[1]);
}

int
setcchars(char *cc, char *s) {
  int c;
  Map *m;

  if(!strcmp(cc, "min") || !strcmp(cc, "time"))
    c = atoi(s);
  else if(!strcmp(s, "^-") || !strcmp(s, "undef"))
    c = _POSIX_VDISABLE;
  else if(s[0] == '^')
    c = CTRL(s[1]);
  else
    c = *s;
  for(m = cchar; m->key; m++)
    if(!strcmp(cc, m->key)) {
      tc.c_cc[m->val] = (uchar)c;
      return 1;
    }
  return 0;
}

int
setcomb(char *s) {
  Map *m;

  if(strchr(s, ':')) {
    readsaved(s);
  } else if(!strcmp(s, "evenp") || !strcmp(s, "parity")) {
    tc.c_cflag &= ~(PARODD|CSIZE);
    tc.c_cflag |= PARENB|CS7;
  } else if(!strcmp(s, "oddp")) {
    tc.c_cflag &= ~CSIZE;
    tc.c_cflag |= PARENB|PARODD|CS7;
  } else if(!strcmp(s, "-evenp") || !strcmp(s, "-parity") ||
    !strcmp(s, "-oddp")) {
    tc.c_cflag &= ~(PARENB|CSIZE);
    tc.c_cflag |= CS8;
  } else if(!strcmp(s, "nl")) {
    tc.c_iflag &= ~ICRNL;
  } else if(!strcmp(s, "-nl")) {
    tc.c_iflag |= ICRNL;
    tc.c_iflag &= ~(INLCR|IGNCR);
  } else if(!strcmp(s, "ek")) {
    tc.c_cc[VERASE] = CTRL('?');
    tc.c_cc[VKILL] = CTRL('U');
  } else if(!strcmp(s, "sane")) {
    memset(&tc, 0, sizeof tc);
    tc.c_cflag |= CS8;
    for(m = control; m->key; m++)
      tc.c_cflag |= m->def;
    /* Linux utf-8 hack */
    tc.c_iflag |= IUTF8;
    for(m = input; m->key; m++)
      tc.c_iflag |= m->def;
    for(m = local; m->key; m++)
      tc.c_lflag |= m->def;
    /* ONLCR is XSI but needed on Linux */
    tc.c_oflag |= ONLCR;
    for(m = output; m->key; m++)
      tc.c_oflag |= m->def;
    for(m = cchar; m->key; m++)
      tc.c_cc[m->val] = m->def;
    cfsetispeed(&tc, B38400);
    cfsetospeed(&tc, B38400);
  } else {
    return 0;
  }
  return 1;
}

int
setflags(char *s, Map *m, uint *flag) {
  int disable;

  disable = 0;
  if(*s == '-') {
    disable = 1;
    s++;
  }
  for(; m->key; m++)
    if(!strcmp(m->key, s)) {
      if(disable)
        *flag &= ~m->val;
      else
        *flag |= m->val;
      return 1;
    }
  return 0;
}

int
setsize(char *s) {
  Map *m;

  for(m = csize; m->key; m++)
    if(!strcmp(s, m->key)) {
      tc.c_cflag &= ~CSIZE;
      tc.c_cflag |= m->val;
      return 1;
    }
  return 0;
}

int
setspeed(char *s, char *p) {
  uint speed;

  speed = getbaudval(p);
  if(speed == (uint)-1) {
    alert("invalid baud rate %s", p);
    rval = 1;
    return 0;
  }
  if(s == nil) {
    cfsetispeed(&tc, speed);
    cfsetospeed(&tc, speed);
  } else if(!strcmp(s, "ispeed")) {
    if(speed == 0)
      speed = cfgetospeed(&tc);
    cfsetispeed(&tc, speed);
  } else if(!strcmp(s, "ospeed")) {
    if(speed == 0)
      speed = cfgetispeed(&tc);
    cfsetospeed(&tc, speed);
  } else {
    return 0;
  }
  return 1;
}

int
main(int argc, char *argv[]) {
  int i;

  setlocale(LC_ALL, "");
  __fixargv0(argv[0]);
  argc--, argv++;
  aflag = gflag = 0;
  if(argc > 0) {
    if(!strcmp(argv[0], "-a"))
      aflag = 1;
    else if(!strcmp(argv[0], "-g"))
      gflag = 1;
  }
  if(aflag+gflag)
    argc--, argv++;

  if((aflag+gflag) && argc > 0) {
    fprint(stderr, "usage: stty [-a|-g]\n"
      "       stty operand...\n");
    exit(1);
  }
  if(tcgetattr(0, &tc))
    fatal(1, "not a tty");
  if(argc == 0) {
    if(gflag) {
      putsaved();
      exit(0);
      /* not reached */
    }
    putspeed();
    putcchars();
    putline(nil);
    putsize();
    putflags(control, &tc.c_cflag);
    putflags(input, &tc.c_iflag);
    putflags(local, &tc.c_lflag);
    putflags(output, &tc.c_oflag);
    putline(nil);
    exit(0);
    /* not reached */
  }
  rval = 0;
  for(i = 0; i < argc; i++) {
    if(setcomb(argv[i]) ||
      setflags(argv[i], control, &tc.c_cflag) ||
      setflags(argv[i], input, &tc.c_iflag) ||
      setflags(argv[i], local, &tc.c_lflag) ||
      setflags(argv[i], output, &tc.c_oflag) ||
      setsize(argv[i]) ||
      (isdigit(argv[i][0]) && setspeed(nil, argv[i])))
      continue;
    if(argv[i+1] && (setspeed(argv[i], argv[i+1]) ||
      setcchars(argv[i], argv[i+1]))) {
      i++;
      continue;
    }
    alert("invalid operand %s", argv[i]);
    rval = 1;
  }
  if(tcsetattr(0, TCSANOW, &tc))
    fatal(1, "can't set attributes: %m");
  exit(rval);
}
