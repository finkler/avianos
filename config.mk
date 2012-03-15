INCS = -I${ROOT}/include
LIBS = -L${ROOT}/lib -lc -lavian

CFLAGS = -std=c99 -pedantic -Wall -Os -D_XOPEN_SOURCE=700 ${INCS}
LDFLAGS = -s -static ${LIBS}

CC = musl-gcc
AR = ar
