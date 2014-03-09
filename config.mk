INCS = -I${ROOT}/include
LIBS = -L${ROOT}/lib -lc -lavian

#CFLAGS = -std=gnu99 -pedantic -Wall -Os ${INCS}
CFLAGS = -std=c99 -pedantic -Wall -Os -D_XOPEN_SOURCE=700 ${INCS}
LDFLAGS = -s -static ${LIBS}

CC = /home/tkr/.local/bin/musl-gcc
AR = ar
