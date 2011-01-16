CC = gcc
CFLAGS = -W -Wall -g -O2 -std=c99 -pedantic
LIBS = -lm -lao
SRCS = aop.c note2freq.c play_note.c
OBJS = $(SRCS:.c=.o)

all: aop

aop: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

test_note2freq: note2freq.c
	$(CC) $(CFLAGS) -DTESTCODE -o $@ note2freq.c $(LIBS)

clean:
	rm -f $(OBJS) aop test_note2freq
