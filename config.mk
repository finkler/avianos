INCS = -I${ROOT}/include
LIBS = -L${ROOT}/lib -lc -lavian

#CFLAGS = -std=c99 -pedantic -Wall -Os -D_XOPEN_SOURCE=700 ${INCS}
CFLAGS = -g -std=c99 -pedantic -Wall -Wfatal-errors -Os -D_XOPEN_SOURCE=700 ${INCS}
#LDFLAGS = -s -static ${LIBS}
LDFLAGS = -static ${LIBS}

CC = musl-gcc
AR = ar
