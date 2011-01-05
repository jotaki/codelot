CC= gcc
CFLAGS= -Wall -g -O2
SRCS= zpas.c instructions.c misc.c define.c zpas_instructions.c
HEADERS= define.h zpas.h
OBJS= $(SRCS:.c=.o)
all: zpas

zpas: $(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	rm $(OBJS) zpas

test:
	@./zpas test.asm /dev/stdout | ndisasm -
	@echo "----------"
	@nasm -f bin -o /dev/stdout test.asm | ndisasm -

diff:
	@./zpas test.asm test.zpas
	@nasm -f bin -o test.nasm test.asm
	@diff --brief test.zpas test.nasm
	@rm test.zpas test.nasm
