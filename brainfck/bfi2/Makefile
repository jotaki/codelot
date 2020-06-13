CC= gcc
CFLAGS= -W -Wall -g -O2 -std=gnu99
SRCS= main.c brainfuck.c machine.c cli.c interface.c misc.c
OBJS= $(SRCS:.c=.o)
LIBS= -lm

all: bfi2

bfi2: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	rm -f bfi2 $(OBJS)
