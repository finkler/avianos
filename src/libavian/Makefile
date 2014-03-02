ROOT=../..
include ${ROOT}/config.mk

SRC = __fixargv0.c bg.c cleanname.c fatal.c fgetln.c ltrim.c readcons.c rtrim.c symmod.c textwidth.c vstradd.c
OBJ = ${SRC:.c=.o}

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

avian: ${OBJ}
	@echo AR lib$@.a
	@${AR} rcs lib$@.a ${OBJ}

clean:
	@echo cleaning
	@rm -f ${OBJ} libavian.a
  
install:
	@echo installing
	@cp libavian.a ${ROOT}/lib

.PHONY: clean install
