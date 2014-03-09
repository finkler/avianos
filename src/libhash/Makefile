ROOT=../..
include ${ROOT}/config.mk

SRC = crc32.c sha1.c
OBJ = ${SRC:.c=.o}

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

hash: ${OBJ}
	@echo AR lib$@.a
	@${AR} rcs lib$@.a ${OBJ}

clean:
	@echo cleaning
	@rm -f ${OBJ} libhash.a
  
install:
	@echo installing
	@cp libhash.a ${ROOT}/lib

.PHONY: clean install
