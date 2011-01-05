CC= gcc
AR= ar
CFLAGS= -W -Wall -s -O2 -fPIC -I. -I/usr/include/mysql -DEBUG
LIBS= -lzpsk -lz -lmysqlclient
SRCS= irc.c irc_cmd.c zebot.c m_cfg.c
OBJS= $(SRCS:.c=.o)

all: zebot

zebot: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
	$(AR) r irc.a irc.o irc_cmd.o m_cfg.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	rm -f $(OBJS) zebot irc.a
