CC= gcc
CFLAGS= -W -Wall -s -O2
SRCS= cddb.c cdrom.c gui.c ncdplay.c
LIBS= -lcddb -lncurses
OBJS= $(SRCS:.c=.o)

all: ncdplay

ncdplay: $(OBJS)
	$(CC) $(CFLAGS) -o ../$@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<

clean:
	rm -f $(OBJS) ../ncdplay
