#ifndef _UTF8_H_
#define _UTF8_H_

#define RUNE_ERROR 0xFFFD
#define RUNE_SELF  0x80
#define UTF_MAX    3

typedef uint16 rune;

int runedec(rune *, char *);
int runeenc(char *, rune);
int runelen(rune);
int runestart(int);
uint utflen(char *);
uint utfnlen(char *, int);
char *utfrune(char *, rune);
char *utfrrune(char *, rune);
#define utfutf strstr

#endif /* _UTF8_H_ */
