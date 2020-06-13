#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>

#include "bfi2.h"

void clr() { printf("\033[2J"); fflush(stdout); }
void moveto(int y, int x) { printf("\033[%d;%dH",y,x); fflush(stdout); }

void termsize(struct winsize *wsp)
{
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, wsp) < 0)
		perror("ioctl(get_win_size)");
}

void raw(bool enable, bool noecho)
{
	static struct termios orig;
	struct termios new;

	if(enable) {
		tcgetattr(STDIN_FILENO, &orig);
		new = orig;
		new.c_lflag &= ~ICANON;

		if(noecho)
			new.c_lflag &= ~ECHO;

		tcsetattr(STDIN_FILENO, TCSAFLUSH, &new);
	} else
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

void showhelp(void)
{
	int y, x;

	clr();

	y = 2;
	x = 74 / 4;

	moveto(y, x);
	printf("+--------------------------------------------------------+\n");

	moveto(y+1, x);
	printf("|                                                        |\n");

	moveto(y+2, x);
	printf("|                 +    : increment value                 |\n");
	
	moveto(y+3, x);
	printf("|                 -    : decrement value                 |\n");
	
	moveto(y+4, x);
	printf("|                 < h  : move left                       |\n");

	moveto(y+5, x);
	printf("|                 > l  : move right                      |\n");

	moveto(y+6, x);
	printf("|                 ,    : get input                       |\n");

	moveto(y+7, x);
	printf("|                 .    : print output                    |\n");

	moveto(y+8, x);
	printf("|                 [    : start loop                      |\n");

	moveto(y+9, x);
	printf("|                 ]    : end loop                        |\n");

	moveto(y+10, x);
	printf("|                 j    : > * 16                          |\n");

	moveto(y+11, x);
	printf("|                 k    : < * 16                          |\n");

	moveto(y+12, x);
	printf("|               n<op>  : do <op> n times                 |\n");

	moveto(y+13, x);
	printf("|                 ?    : show this help.                 |\n");

	moveto(y+14, x);
	printf("|                                                        |\n");

	moveto(y+15, x);
	printf("|          Press any key to return to interface.         |\n");

	moveto(y+16, x);
	printf("|                                                        |\n");

	moveto(y+17, x);
	printf("+--------------------------------------------------------+\n");

	fgetc(stdin);
}

void drawheader(struct interface *ifacep, struct machine *machinep)
{
	(void) ifacep;

	// Reverse and bold text.
	printf("\033[7m\033[1m");

	// print instruction pointer
	printf("ip: %08x", machinep->ip);

	// padding
	for(int i = 0; i < FIXED_LINE_WIDTH - 24; ++i)
		printf(" ");

	// print memory pointer
	printf("mp: %08x\n", machinep->memp);

	// reset video
	printf("\033[0m");
}

void drawseparator(struct interface *ifacep)
{
	moveto(ifacep->ws.ws_row / 2, 0);

	// reverse and bold
	printf("\033[7m\033[1m");

	// print a bar
	for(int i = 0; i < FIXED_LINE_WIDTH; ++i)
		printf("\u2500");

	// reset video
	printf("\033[0m");
}

void drawfooter(struct interface *ifacep, struct machine *machinep)
{
	(void) machinep;

	moveto(ifacep->ws.ws_row, 0);

	// reverse and bold video
	printf("\033[7m\033[1m");

	// pad
	for(int i = 0; i < FIXED_LINE_WIDTH; ++i)
		printf(" ");

	// reset video
	printf("\033[0m");
}

// output is
//   memaddr hex hex hex hex .... |bbbbb...|
//   memaddr hex hex hex hex .... |bbbbb...|
//   memaddr hex hex hex hex .... |bbbbb...|
//   memaddr hex hex hex hex .... |bbbbb...|
//   memaddr hex hex hex hex .... |bbbbb...|
void showmemory(struct interface *ifacep, struct machine *machinep)
{
	moveto(2, 0);

	// calculate max bytes on screen.
	int max = (ifacep->ws.ws_row / 2 - 2) * FIXED_MEM_WIDTH;

	// no space to print
	if(max <= 0) return;

	// loop memory
	for(int offset = 0; offset < max; offset += FIXED_MEM_WIDTH) {
		// print address
		printf("%08x ", ifacep->startaddr + offset);

		// print hex value at address
		for(int i = 0; i < FIXED_MEM_WIDTH; ++i) {
			int addr = ifacep->startaddr + offset + i;
			printf(" %02x", machinep->memory[addr] & 0xff);
		}

		printf("  |");
		
		// print byte values at address
		for(int i = 0; i < FIXED_MEM_WIDTH; ++i) {
			int addr = ifacep->startaddr + offset + i;
			int ch = machinep->memory[addr];
			ch = isprint(ch) ? ch : '.';
			printf("%c", ch);
		}

		printf("|\n");
	}
}

void updatestartaddr(struct interface *ifacep, struct machine *machinep)
{
	// max bytes
	int max = (ifacep->ws.ws_row / 2 - 2) * FIXED_MEM_WIDTH;

	if(machinep->memp < ifacep->startaddr) {
		// scroll up
		
		ifacep->startaddr = machinep->memp;
		ifacep->startaddr -= (machinep->memp % FIXED_MEM_WIDTH);

	} else if(machinep->memp >= ifacep->startaddr + max) {
		// scroll down

		while(ifacep->startaddr + max <= machinep->memp)
			ifacep->startaddr += FIXED_MEM_WIDTH;
	}
}

void updatecursor(struct interface *ifacep, struct machine *machinep)
{
	// calculate distance between (display) start address and memp
	int dist = machinep->memp - ifacep->startaddr;

	// calculate y
	int y = dist / FIXED_MEM_WIDTH + 2;

	// calculate x
	int x = 8 + 3 * ((dist % FIXED_MEM_WIDTH) + 1);

	// move cursor to calculated y,x position.
	moveto(y, x);
}

int userinput(struct interface *ifacep, struct machine *machinep, int ch)
{
	static int repeat = 0;
	(void) ifacep;

	if(!isdigit(ch) && repeat == 0)
		repeat = 1;

	switch(ch) {
		// quit app
		case 'q':
			return 1;
			break;

		// move right
		case '>':
		case 'l':
			for(int rep = 0; rep < repeat; ++rep) {
				brainfuck_eval_chr(machinep, '>', ifacep->execute);
				ifacep->code.buf[ifacep->code.length++] = '>';
			}
			break;

		// move left
		case '<':
		case 'h':
			for(int rep = 0; rep < repeat; ++rep) {
				brainfuck_eval_chr(machinep, '<', ifacep->execute);
				ifacep->code.buf[ifacep->code.length++] = '<';
			}
			break;

		// increment and decrement
		case '+':
		case '-':
			for(int rep = 0; rep < repeat; ++rep) {
				brainfuck_eval_chr(machinep, ch, ifacep->execute);
				ifacep->code.buf[ifacep->code.length++] = ch;
			}

			break;

		// move right by 16
		case 'j':
			for(int rep = 0; rep < repeat; ++rep) {
				for(int i = 0; i < FIXED_MEM_WIDTH; ++i) {
					brainfuck_eval_chr(machinep, '>', ifacep->execute);
					ifacep->code.buf[ifacep->code.length++] = '>';
				}
			}

			break;

		// move left by 16
		case 'k':
			for(int rep = 0; rep < repeat; ++rep) {
				for(int i = 0; i < FIXED_MEM_WIDTH; ++i) {
					brainfuck_eval_chr(machinep, '<', ifacep->execute);
					ifacep->code.buf[ifacep->code.length++] = '<';
				}
			}
			break;

		// move to beginning of 16 byte boundry
		case '^':
		{
			int boundry = machinep->memp;
			boundry %= FIXED_MEM_WIDTH;

			for(int i = 0; i < boundry; ++i) {
				brainfuck_eval_chr(machinep, '<', ifacep->execute);
				ifacep->code.buf[ifacep->code.length++] = '<';
			}
		}
		break;

		// move to end of 16 byte boundry
		case '$':
		{
			int boundry = machinep->memp;
			boundry %= FIXED_MEM_WIDTH;
			boundry = FIXED_MEM_WIDTH - boundry - 1;

			for(int i = 0; i < boundry; ++i) {
				brainfuck_eval_chr(machinep, '>', ifacep->execute);
				ifacep->code.buf[ifacep->code.length++] = '>';
			}
		}
		break;

		// shorthand for [-]
		case '_':
			brainfuck_compile(machinep, "[-]");
			if(ifacep->execute) machine_run(machinep);
			else                machine_skip(machinep);

			ifacep->code.length += 
				sprintf(&ifacep->code.buf[ifacep->code.length], "[-]");

			break;

		// putc
		case '.':
			brainfuck_eval_chr(machinep, ch, ifacep->execute);
			ifacep->code.buf[ifacep->code.length++] = '.';
			break;

		// begin loop if memory[memp] != 0
		case '[':
			if(machinep->memory[machinep->memp] == 0)
				ifacep->execute = false;
			else
				ifacep->execute = true;

			brainfuck_eval_chr(machinep, ch, false);
			ifacep->code.buf[ifacep->code.length++] = '[';

			// store ip of loop beginning
			ifacep->loopaddr[ifacep->curloop++] = machinep->ip - 1;
			break;

		case ']': {
			ifacep->code.buf[ifacep->code.length++] = ']';

			// get current loop
			int curloop = --ifacep->curloop;

			// hacky- should this be here?
			// get starting ip
			int startip = ifacep->loopaddr[curloop];

			// update matching '[' with newly placed ']'.
			machinep->ops[startip].param.addr = machinep->codesize;
			machinep->ops[machinep->codesize].param.addr = startip;

			int memp = machinep->memp;

			// execute code if necessary.
			if(ifacep->curloop == 0 || machinep->memory[memp] != 0)
				ifacep->execute = true;

			brainfuck_eval_chr(machinep, ']', ifacep->execute);
		}
		break;

		default:
			if(isdigit(ch))
				repeat = repeat * 10 + (ch - '0');
	}

	if(!isdigit(ch))
		repeat = 0;

	return 0;
}

void cliread(struct interface *ifacep, struct machine *machinep)
{
	int ch;

	(void) machinep;

	if(read(ifacep->inputfd, &ch, 1) < 0) {
		interface_appendoutput(ifacep, "couldn't read input.");
		return;
	}

	interface_appendoutput(ifacep, "%c", ch);
}

void showoutput(struct interface *ifacep, struct machine *machinep)
{
	(void) machinep;

	int offset = 0;

	// move to output canvas
	moveto(ifacep->ws.ws_row / 2 + 1, 0);

	// calculate max
	int max = (ifacep->ws.ws_row - 1) - (ifacep->ws.ws_row / 2 + 1);
	max *= FIXED_LINE_WIDTH;

	if(ifacep->output.length > max)
		offset = ifacep->output.length - max;

	// output buffer to canvas.
	for(int j = 0, i = offset; i < ifacep->output.length; ++i, ++j) {
		printf("%c", ifacep->output.buf[i]);

		if(ifacep->output.buf[i] == '\n')
			j = 0;

		if(j % FIXED_LINE_WIDTH == 0 && j > 0)
			printf("\n");
	}
}

void showcode(struct interface *ifacep, struct machine *machinep)
{
	(void) machinep;

	// calculate length
	int length = ifacep->code.length;
	int offset = 0;

	if(length > FIXED_CODE_LENGTH) {
		offset = length - FIXED_CODE_LENGTH;
		length = FIXED_CODE_LENGTH;
	}

	// calculate center
	int centerx = (FIXED_LINE_WIDTH / 2) - (length / 2);

	// move to center of bottom row
	moveto(ifacep->ws.ws_row, centerx);

	// ensure reverse video and bold.
	printf("\033[7m\033[1m");

	// print code
	for(int i = 0; i < length; ++i)
		printf("%c", ifacep->code.buf[i+offset]);

	// reset video
	printf("\033[0m");
}

int cli(struct machine *machinep)
{
	struct interface *iface = NULL;
	int infd[2], outfd[2], rc = 0;

	if(!interface_create(&iface)) {
		perror("could not create interface");
		return 1;
	}

	if(pipe(infd) < 0) {
		perror("failed to create pipe");
		rc = 1;
		goto iface_destroy;
	}

	if(pipe(outfd) < 0) {
		perror("failed to create pipe");
		rc = 1;
		goto infd_close;
	}

	// use pipes for io buffer
	machinep->inputfd = infd[0];
	machinep->outputfd = outfd[1];
	
	iface->execute = true;
	iface->inputfd = outfd[0];
	iface->outputfd = infd[1];

	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(iface->inputfd, &readfds);
	FD_SET(fileno(stdin), &readfds);

	// enter raw mode
	raw(true, true);

	while(true) {
		termsize(&iface->ws);
		clr();
		moveto(0, 0);

		drawheader(iface, machinep);
		drawseparator(iface);
		drawfooter(iface, machinep);

		updatestartaddr(iface, machinep);

		showmemory(iface, machinep);
		showoutput(iface, machinep);
		showcode(iface, machinep);

		updatecursor(iface, machinep);

		fd_set cpy = readfds;
		int nfds = MAX(fileno(stdin), iface->inputfd) + 1;
		int ret = select(nfds, &cpy, NULL, NULL, NULL);
		if(ret < 0) {
			interface_appendoutput(iface, "select failed.");
			continue;
		} else if(ret > 0) {
			if(FD_ISSET(fileno(stdin), &cpy)) {
				int ch = fgetc(stdin);
				if(userinput(iface, machinep, ch))
					break;

			} else if(FD_ISSET(iface->inputfd, &cpy))
				cliread(iface, machinep);
		}
	}

	// leave raw mode
	raw(false, true);

	// clear screen
	clr();
	moveto(0,0);

//outfd_close:
	close(outfd[0]);
	close(outfd[1]);

infd_close:
	close(infd[0]);
	close(infd[1]);

iface_destroy:
	interface_destroy(&iface);

	return rc;
}
