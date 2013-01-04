#include <u.h>
#include <avian.h>

#define INCR  100

typedef struct Edge Edge;
struct Edge {
  char *u, *v;
  Edge *next;
};

int cap, len;
char **V;
Edge *E;

void
insertedge(char *u, char *v) {
  Edge *tm, *r;

  tm = malloc(sizeof(Edge));
  tm->u = strdup(u);
  tm->v = strdup(v);
  tm->next = nil;
  if(E == nil) {
    E = tm;
  } else {
    r = E;
    while(r->next)
      r = r->next;
    r->next = tm;
  }
}

int
isalreadythere(char *v) {
  int i;

  for(i = 0; i < len; i++)
    if(!strcmp(V[i], v))
      return 1;
  return 0;
}

void
parsefile(char *s) {
  char *buf, *p1, *p2;
  char *delim;
  Edge *r;

  E = nil;
  delim = " \t\n";
  while((buf = fgetln(stdin))) {
    p1 = strtok(buf, delim);
    p2 = strtok(nil, delim);
    while(p1 && p2) {
      insertedge(p1, p2);
      p1 = strtok(nil, delim);
      p2 = strtok(nil, delim);
    }
    if(p1 && !p2)
      fatal(1, "odd number of items");
  }
  if(ferror(stdin))
    fatal(1, "error reading %s: %m", s);
  cap = INCR;
  len = 0;
  V = malloc(cap*sizeof(char *));
  for(r = E; r; r = r->next) {
    if(len == cap) {
      cap += INCR;
      V = realloc(V, cap*sizeof(char *));
    }
    if(!isalreadythere(r->u))
      V[len++] = r->u;
    if(!isalreadythere(r->v))
      V[len++] = r->v;
  }
}

void
removeedge(char *s) {
  char *p;
  Edge *r1, *r2;

  p = strdup(s);
  r1 = E;
  while(r1 && (r2 = r1->next))
    if(!strcmp(r2->u, p)) {
      r1->next = r2->next;
      free(r2->u);
      free(r2->v);
      free(r2);
    } else {
      r1 = r1->next;
    }
  if(E && !strcmp(E->u, p)) {
    r1 = E->next;
    free(E->u);
    free(E->v);
    free(E);
    E = r1;
  }
  free(p);
}

void
usage(void) {
  fprint(stderr, "usage: tsort [file]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int cyclic, i, j;
  Edge *r;

  ARGBEGIN(""){
  default:
    usage();
  }ARGEND 
  
  if(argc > 1)
    usage();
  if(argc == 1 && !freopen(argv[0], "r", stdin))
    fatal(1, "can't open %s: %m", argv[0]);
  parsefile(argc == 1 ? argv[0] : "<stdin>");
  while(len) {
    cyclic = 1;
    i = 0;
    while(i < len) {
      for(r = E; r; r = r->next)
        if(strcmp(V[i], r->u) && !strcmp(V[i], r->v))
          break;
      if(r) {
        i++;
        continue;
      }
      println(V[i]);
      removeedge(V[i]);
      len--;
      for(j = i; j < len; j++)
        V[j] = V[j+1];
      cyclic = i = 0;
    }
    if(cyclic)
      fatal(1, "not a directed acyclic graph");
  }
}
