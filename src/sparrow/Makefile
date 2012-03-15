ROOT = ../..
include ${ROOT}/config.mk

TARG = _

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${TARG}: ${TARG}.o
	@echo LD $@
	@${CC} -o $@ $< ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f *.o

.PHONY: clean
