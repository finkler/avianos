#include <u.h>
#include <utf8.h>

char *
utfrune(char *s, rune r) {
  char buf[UTF_MAX];

  runeenc(buf, r);
  return strstr(s, buf);
}