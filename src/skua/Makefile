ROOT = ../..
include ${ROOT}/config.mk

TARG = _

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${TARG}: ${TARG}.o
	@echo LD $@
	@${CC} -o $@ $< ${LDFLAGS} ${EXTLIB}

clean:
	@echo cleaning
	@rm -f *.o

.PHONY: clean
