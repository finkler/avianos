ROOT=../..
include ${ROOT}/config.mk

SRC = runedec.c runeenc.c runelen.c runestart.c utflen.c utfnlen.c utfrrune.c utfrune.c
OBJ = ${SRC:.c=.o}

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

utf8: ${OBJ}
	@echo AR lib$@.a
	@${AR} rcs lib$@.a ${OBJ}

clean:
	@echo cleaning
	@rm -f ${OBJ} libutf8.a

install:
	@echo installing
	@cp libutf8.a ${ROOT}/lib

.PHONY: clean install
