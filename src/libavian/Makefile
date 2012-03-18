ROOT=../..
include ${ROOT}/config.mk

SRC = cleanname.c daemon.c fatal.c fgetln.c getmode.c readcons.c textwidth.c sysfatal.c
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
