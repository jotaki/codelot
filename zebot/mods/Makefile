CC= gcc
CFLAGS= -W -Wall -s -O2 -I. -I../ -I/usr/include/mysql -fPIC
ADDONS= ../irc.a -lmysqlclient

all:
	@echo "Sorry, please specify a module to make."
	@exit 1

default:
	$(CC) $(CFLAGS) -c default.c -o default.o
	$(CC) $(CFLAGS) -shared -o default.so default.o $(ADDONS)

n:
	$(CC) $(CFLAGS) -c n.c -o n.o
	$(CC) $(CFLAGS) -shared -o n.so n.o $(ADDONS)

clean:
	rm -f *.o *.so
