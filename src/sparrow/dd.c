#include <u.h>
#include <avian.h>
#include <signal.h>

#define LCASE 1
#define UCASE 2
#define SWAB 4
#define NOERROR 8
#define NOTRUNC 16
#define SYNC 32

#define CONV(f) ((conv&f)==f)

int  ccase(int);
void cpblk(char *, char *, int, int *, int *);
void cpraw(char *, char *, int, int *, int *);
void cpubl(char *, char *, int, int *, int *);
void dd(void);
uint expr(char *);
void summary(int);
void swap(char *, int);

int again;
char *fin, *fout;
void (*copy)(char *, char *, int, int *, int *);
uint cbs, conv, count, ibs, obs, skip, seek;
uint ibc, ibp, obc, obp, tbc;

int
ccase(int c) {
  if(CONV(LCASE))
    return tolower(c);
  if(CONV(UCASE))
    return toupper(c);
  return c;
}

void
cpblk(char *bin, char *bout, int n, int *nr, int *nw) {
  static int eol, ic, ii, io;
  int c, i, j, m;

  m = obs - io;
  for(i = j = 0; !eol && i < n && j < m; ic++) {
    c = ccase(bin[ii++]);
    i++;
    if(c == '\n' || (i == n && ibp)) {
      eol = 1;
      break;
    }
    if(ic < cbs) {
      bout[io++] = c;
      j++;
    }
  }
  if(i == n)
    ii = 0;
  *nr = i;
  if(eol) {
    if(ic > cbs) {
      ic = cbs;
      tbc++;
    } else {
      while(ic < cbs && j < m) {
        bout[io++] = ' ';
        ic++, j++;
      }
    }
    if(ic == cbs)
      eol = ic = 0;
    else if(i == n && ibp)
      again = 1;
  }
  if(j == m)
    io = 0;
  *nw = j;
}

void
cpraw(char *bin, char *bout, int n, int *nr, int *nw) {
  static int ii, io;
  int i, m;

  m = obs - io;
  for(i = 0; i < n && i < m; i++)
    bout[io++] = ccase(bin[ii++]);
  if(i == n)
    ii = 0;
  if(i == m)
    io = 0;
  *nr = *nw = i;
}

void
cpubl(char *bin, char *bout, int n, int *nr, int *nw) {
  static int ic, ii, io;
  int i, m;

  m = obs - io;
  for(i = 0; ic < cbs && i < n && i < m; i++, ic++)
    bout[io++] = ccase(bin[ii++]);
  if(i == n)
    ii = 0;
  *nr = i;
  if(ic == cbs || (i == n && ibp)) {
    while(io > 0 && isspace(bout[io-1]))
      i--, io--;
    if(i < m) {
      bout[io++] = '\n';
      i++;
      ic = 0;
    } else if(*nr == n && ibp)
      again = 1;
  }
  if(i == m)
    io = 0;
  *nw = i;
}

void
dd(void) {
  char bin[ibs], bout[obs];
  uint cnt;
  int flags, ifd, ina, nr, nw, ofd, ona, pad;

  if(fin) {
    ifd = open(fin, O_RDONLY);
    if(ifd < 0)
      fatal(1, "open %s: %m", fin);
  } else {
    fin = "<stdin>";
    ifd = 0;
  }
  if(fout) {
    flags = O_CREAT;
    if(seek) {
      flags |= O_RDWR;
    } else {
      flags |= O_WRONLY;
      if(!CONV(NOTRUNC))
        flags |= O_TRUNC;
    }
    ofd = open(fout, flags, 0644);
    if(ofd < 0)
      fatal(1, "open %s: %m", fout);
  } else {
    fout = "<stdout>";
    ofd = 1;
  }
  while(seek--) {
    nr = read(ofd, bout, obs);
    if(nr < 0)
      fatal(1, "read %s: %m", fout);
    if(nr < obs) {
      memset(bout+nr, 0, obs-nr);
      nw = write(ofd, bout, obs);
      if(nw != obs)
        fatal(1, "write %s: %m", fout);
    }
  }
  again = pad = 0;
  cnt = ina = ona = 0;
  ibc = ibp = obc = obp = tbc = 0;
  if(copy != cpraw)
    pad = ' ';
  while(count == 0 || cnt < count) {
    nr = read(ifd, bin, ibs);
    if(nr == 0 && ona == 0)
      break;
    if(nr < 0) {
      alert("read %s: %m", fin);
      summary(0);
      if(!CONV(NOERROR) && ona == 0)
        exit(1);
      nr = 0;
    }
    if(skip) {
      skip--;
      continue;
    }
    cnt++;
    if(nr == ibs)
      ibc++;
    else
      ibp++;
    if(CONV(SYNC))
      while(nr < ibs)
        bin[nr++] = pad;
    if(CONV(SWAB))
      swap(bin, nr);
    ina = nr;
  Copy:
    copy(bin, bout, ina, &nr, &nw);
    ina -= nr;
    ona += nw;
    if(ona < obs) {
      if(ina > 0)
        goto Copy;
      if(!ibp)
        continue;
    }
    nw = write(ofd, bout, ona);
    if(nw < 0)
      fatal(1, "write %s: %m", fout);
    ona = 0;
    if(nw == obs)
      obc++;
    else
      obp++;
    if(ina > 0 || again) {
      again = 0;
      goto Copy;
    }
  }
}

uint
expr(char *s) {
  uint n;

  n = strtoul(s, &s, 10);
  switch(*s++) {
  case 'b':
    n *= 512;
    break;
  case 'k':
    n *= 1024;
    break;
  case 'x':
    n *= expr(s);
    break;
  case '\0':
    break;
  default:
    fatal(1, "invalid expression");
  }
  return n;
}

void
summary(int sig) {
  fprintf(stderr, "%u+%u records in\n"
    "%u+%u records out\n", ibc, ibp, obc, obp);
  if(tbc > 1)
    fprintf(stderr, "%u truncated records\n", tbc);
  else if(tbc)
    fprintf(stderr, "%u truncated record\n", tbc);
  if(sig == SIGINT)
    exit(1);
}

void
swap(char *bin, int n) {
  int c, i;

  for(i = 0; i < n-1; i += 2) {
    c = bin[i];
    bin[i] = bin[i+1];
    bin[i+1] = c;
  }
}

int
main(int argc, char *argv[]) {
  int i;
  char *p, *q;

  ARGBEGIN(""){
  default:
    fprint(stderr, "usage: dd [operand...]\n");
    exit(1);
  }ARGEND

  fin = fout = nil;
  cbs = conv = count = seek = skip = 0;
  copy = cpraw;
  ibs = obs = 512;
  for(i = 0; i < argc; i++) {
    p = strchr(argv[i], '=');
    if(p == nil)
      fatal(1, "%s: invalid operand", argv[i]);
    *p++ = '\0';
    if(!strcmp(argv[i], "if")) {
      fin = p;
    } else if(!strcmp(argv[i], "of")) {
      fout = p;
    } else if(!strcmp(argv[i], "ibs")) {
      ibs = expr(p);
    } else if(!strcmp(argv[i], "obs")) {
      obs = expr(p);
    } else if(!strcmp(argv[i], "bs")) {
      ibs = obs = expr(p);
    } else if(!strcmp(argv[i], "cbs")) {
      cbs = expr(p);
    } else if(!strcmp(argv[i], "skip")) {
      skip = strtoul(p, nil, 10);
    } else if(!strcmp(argv[i], "seek")) {
      seek = strtoul(p, nil, 10);
    } else if(!strcmp(argv[i], "count")) {
      count = strtoul(p, nil, 10);
    } else if(!strcmp(argv[i], "conv")) {
      for(q = strtok(p, ","); q; q = strtok(nil, ","))
        if(!strcmp(q, "block"))
          copy = cpblk;
        else if(!strcmp(q, "unblock"))
          copy = cpubl;
        else if(!strcmp(q, "lcase"))
          conv |= LCASE;
        else if(!strcmp(q, "ucase"))
          conv |= UCASE;
        else if(!strcmp(q, "swab"))
          conv |= SWAB;
        else if(!strcmp(q, "noerror"))
          conv |= NOERROR;
        else if(!strcmp(q, "notrunc"))
          conv |= NOTRUNC;
        else if(!strcmp(q, "sync"))
          conv |= SYNC;
        else
          fatal(1, "%s: unkown value", q);
    } else {
      fatal(1, "%s: unkown operand", argv[i]);
    }
  }
  signal(SIGINT, summary);
  dd();
  summary(0);
  exit(rval);
}
