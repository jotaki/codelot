CC= gcc
CFLAGS= -W -Wall -s -O2
AR= ar
RANLIB= ranlib
SRCS=sock.c mkshell.c
OBJS=$(SRCS:.c=.o)

all: libtool

mkshell:
	$(CC) $(CFLAGS) -DAS_APP -c mkshell.c
	$(CC) $(CFLAGS) -DAS_APP -o ../mkshell mkshell.o

libtool: $(OBJS)
	$(AR) r ../$@.a $(OBJS)
	$(RANLIB) ../$@.a

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS)
