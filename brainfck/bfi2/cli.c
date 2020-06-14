#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <math.h>

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

void printw(int y, int x, char *str)
{
	moveto(y, x);

	int len = strlen(str);
	int pad = (56 - len) / 2;

	printf("\u2502");
	for(int i = 0; i < pad; ++i) printf(" ");

	printf("%s", str);
	
	for(int i = len + pad + 1; i < 57; ++i) printf(" ");
	printf("\u2502");
}

void showhelp(struct interface *ifacep)
{
	(void) ifacep;
	static char *help[] = {
		"+       : increment value      ",
		"-       : decrement value      ",
		"<  h    : move left            ",
		">  l    : move right           ",
		"",
		"^       : go to beginning of 16",
		"          cell boundry.        ",
		"$       : go to end of 16 cell ",
		"          boundry.             ",
		"_       : zero out cell. short ",
		"          hand for [-]         ",
		"",
		"[       : begin loop if cell   ",
		"          is nonzero           ",
		"]       : jump to beginning    ",
		"          of loop              ",
		"",
		".       : print cell to output ",
		",       : read input to cell   ",
		"",
		"j       : move 16 cells to the ",
		"          right.               ",
		"k       : move 16 cells to the ",
		"          left.                ",
		"",
		"m       : mark cell to register",
		"          available registers: ",
		"          0-9, a-f             ",
		"",
		"M       : move value marked by ",
		"          register 0 to current",
		"          cell.                ",
		"",
		"i       : insert value by      ",
		"          factoring the input  ",
		"          character.           ",
		"",
		"?       : show this help.      ",
	};

	int y, x, offset = 0, ch;

	do {
		y = (ifacep->ws.ws_row - 17) / 2;
		x = (FIXED_LINE_WIDTH - 58) / 2;

		moveto(y, x);

		printf("\u250c");
		for(int i = 0; i < 56; ++i) printf("\u2500");
		printf("\u2510");
		
		printw(++y, x, "");

		for(int i = 0; i < 14; ++i)
			printw(++y, x, help[offset+i]);

		printw(++y, x, "");
		printw(++y, x, "Scroll with j and k, q to close.");

		moveto(++y, x);

		printf("\u2514");
		for(int i = 0; i < 56; ++i) printf("\u2500");
		printf("\u2518");

		moveto(0, 0);
		ch = fgetc(stdin);

		switch(ch) {
			case 'j':
				offset = MIN(offset+1, 38-14);
				break;

			case 'k':
				offset = MAX(offset - 1, 0);
				break;
		}
	} while(ch != 'q');
}

void drawheader(struct interface *ifacep, struct machine *machinep)
{
	// Reverse and bold text.
	printf("\033[7m\033[1m");

	// print instruction pointer
	printf("ip: %08x", machinep->ip);

	// length and center of length
	unsigned int length = FIXED_LINE_WIDTH - 24;
	unsigned int center = length / 2;

	// iterator and iterator max
	unsigned int i, max = 0;

	// display string
	char *dspstr = "";

	switch(ifacep->mode) {
		case IM_DEFAULT:
			max = ifacep->curloop;
			break;

		case IM_FUNC_INSERT:
			max = 6;
			dspstr = "INSERT";
			break;

		case IM_FUNC_MARK:
			max = 4;
			dspstr = "MARK";
			break;

		case IM_INPUT:
			max = 14;
			dspstr = "Awaiting input";
			break;
	}

	// calculate center
	center = length / 2 - max;

	// padding
	for(i = 0; i < center; ++i)
		printf(" ");

	// print '[' times the number of open loops.
	if(ifacep->mode != IM_DEFAULT) {
		printf("%s", dspstr);
		i += max;
	} else {
		for(unsigned int j = 0; j < ifacep->curloop; ++j, ++i)
			printf("[");
	}

	// more padding
	for(; i < length; ++i)
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
			unsigned int addr = ifacep->startaddr + offset + i;
			bool highlight = false;

			for(int i = 0; i < MAX_MARKS; ++i) {
				if((ifacep->mark[i] - 1) == addr) {
					highlight = true;
					break;
				}
			}

			if(!highlight)
				printf(" %02x", machinep->memory[addr] & 0xff);
			else
				printf("\e[7m %02x\e[0m", machinep->memory[addr] & 0xff);
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
	int y = 0, x = 0;

	if(ifacep->mode == IM_INPUT)
		calculateyx(ifacep, &y, &x);
	else {
		// calculate distance between (display) start address and memp
		int dist = machinep->memp - ifacep->startaddr;

		// calculate y
		y = dist / FIXED_MEM_WIDTH + 2;

		// calculate x
		x = 8 + 3 * ((dist % FIXED_MEM_WIDTH) + 1);
	}

	// move cursor to calculated y,x position.
	moveto(y, x);
}

int userinputhelp(struct interface *ifacep, struct machine *machinep, int ch)
{
	(void) machinep;
	(void) ch;

	ifacep->mode = IM_DEFAULT;

	return 0;
}

int userinput(struct interface *ifacep, struct machine *machinep, int ch)
{
	int (*funcp)() = userinputdefault;

	switch(ifacep->mode) {
		case IM_DEFAULT:
			funcp = userinputdefault;
			break;

		case IM_FUNC_INSERT:
			funcp = userinputinsert;
			break;

		case IM_FUNC_MARK:
			funcp = userinputmark;
			break;

		case IM_INPUT:
			funcp = userinputbrainfuck;
			break;
	}

	return funcp(ifacep, machinep, ch);
}

int userinputbrainfuck(struct interface *ifacep, struct machine *machinep, int ch)
{
	interface_appendoutput(ifacep, "%c", ch);
	if(write(ifacep->outputfd, &ch, 1) < 0)
		perror("write");

	brainfuck_eval_chr(machinep, ',', ifacep->execute);

	ifacep->mode = IM_DEFAULT;
	return 0;
}

int userinputmark(struct interface *ifacep, struct machine *machinep, int ch)
{
	ifacep->mode = IM_DEFAULT;

	// default to marker 0.
	if(ch == '\n')
		ch = '0';

	if(isxdigit(ch)) {
		unsigned int index = ch - '0';

		if(ch >= 'a' && ch <= 'f')
			index = ch - 87;
		else if(ch >= 'A' && ch <= 'F')
			index = ch - 55;

		unsigned int memp = ifacep->mark[index];
		if(memp != machinep->memp+1) {
			ifacep->mark[index] = machinep->memp+1;
			if(memp == 0)
				++ifacep->nmarks;
		} else {
			--ifacep->nmarks;
			ifacep->mark[index] = 0;
		}
	} else if(ch == 'm') {
		unsigned int startaddr = machinep->memp - (machinep->memp % 16);
		bool alreadymarked = true;

		for(int i = 0; i < MAX_MARKS; ++i) {
			if(ifacep->mark[i] != startaddr + i + 1) {
				alreadymarked = false;
				break;
			}
		}
		
		for(int i = 0; i < MAX_MARKS; ++i)
			ifacep->mark[i] = alreadymarked ? 0 : startaddr + i + 1;

		ifacep->nmarks = alreadymarked ? 0 : MAX_MARKS;
	}

	return 0;
}

int userinputdefault(struct interface *ifacep, struct machine *machinep, int ch)
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

		// insert value
		case 'i':
			ifacep->mode = IM_FUNC_INSERT;
			break;

		// mark value
		case 'm':
			ifacep->mode = IM_FUNC_MARK;
			break;

		// move character
		case 'M': {
			int memp0 = ifacep->mark[0];
			int mp = machinep->memp;

			if(memp0 == 0) {
				alert(ifacep, "please asign marker 0\n");
				break;
			}

			// fix memp location.
			memp0 -= 1;

			// find marked position
			char *oldcell = calculate_offset(memp0 - mp);
			if(!oldcell)
				break;

			// find cell back
			char *newcell = calculate_inverse(oldcell);
			if(!newcell) {
				free(oldcell);
				break;
			}

			// create buffer.
			int olen = strlen(oldcell);
			int clen = strlen(newcell);

			// 3 + olen + 1 + clen + 1 + olen + 2 + clen + 1
			char *buf = calloc(8 + 2 * olen + 2 * clen, sizeof(char));
			if(!buf) {
				free(oldcell);
				free(newcell);
				break;
			}

			// brainfuck code.
			sprintf(buf,"[-]%s[%s+%s-]%s", oldcell, newcell, oldcell, newcell);

			// compile and run/skip.
			brainfuck_compile(machinep, buf);
			if(ifacep->execute)
				machine_run(machinep);
			else
				machine_skip(machinep);

			// unmark 0.
			ifacep->mark[0] = 0;
			--ifacep->nmarks;

			// cleanup
			free(buf);
			free(newcell);
			free(oldcell);
		}
		break;

		case '?':
			showhelp(ifacep);
			break;

		case ',':
			ifacep->mode = IM_INPUT;
			break;

		default:
			if(isdigit(ch))
				repeat = repeat * 10 + (ch - '0');
	}

	if(!isdigit(ch))
		repeat = 0;

	return 0;
}

int userinputinsert(struct interface *ifacep, struct machine *machinep, int ch)
{
	ifacep->mode = IM_DEFAULT;

	// compute sqrt of entered character.
	int sqr = sqrt(ch);

	// compute factors
	int f0, f1, pad = 0;

	for(f0 = sqr; ch % f0 != 0; --f0)
		/* do nothing */ ;

	// prime case
	if(f0 == 1) {
		f0 = ch / sqr;
		pad = ch % sqr;
	}

	f1 = ch / f0;

	// allocate buffer
	char *buf = calloc(14 + f0 + f1 + pad + 1, sizeof(char));
	if(!buf)
		return 0;

	// start brainfuck
	int length = sprintf(buf, "[-]>[-]");
	
	// outside value
	for(int i = 0; i < f0; ++i)
		buf[length++] = '+';

	// start loop
	buf[length++] = '[';
	buf[length++] = '<';

	// inside value
	for(int i = 0; i < f1; ++i)
		buf[length++] = '+';

	// close loop
	buf[length++] = '>';
	buf[length++] = '-';
	buf[length++] = ']';

	// shift to newly created value.
	buf[length++] = '<';

	// pad if necessary
	for(int i = 0; i < pad; ++i)
		buf[length++] = '+';

	buf[length++] = '\0';

	// compile and run/skip
	brainfuck_compile(machinep, buf);

	if(ifacep->execute)
		machine_run(machinep);
	else
		machine_skip(machinep);

	free(buf);
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

	// move to output canvas
	moveto(ifacep->ws.ws_row / 2 + 1, 0);

	int height = ifacep->ws.ws_row / 2 - 2;
	int maxbytes = height * FIXED_LINE_WIDTH;
	int offset = 0, newlines = 0;

	if(ifacep->output.length > maxbytes)
		offset = ifacep->output.length - maxbytes;

	for(int i = ifacep->output.length; i >= offset; --i) {
		if(ifacep->output.buf[i] == '\n') {
			if(++newlines == height+2) {
				offset = i + 1;
				break;
			}
		}
	}

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

int cliloop(struct interface *ifacep, struct machine *machinep)
{
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(ifacep->inputfd, &readfds);
	FD_SET(fileno(stdin), &readfds);

	termsize(&ifacep->ws);
	clr();
	moveto(0, 0);

	drawheader(ifacep, machinep);
	drawseparator(ifacep);
	drawfooter(ifacep, machinep);

	updatestartaddr(ifacep, machinep);

	showmemory(ifacep, machinep);
	showoutput(ifacep, machinep);
	showcode(ifacep, machinep);

	updatecursor(ifacep, machinep);

	fd_set cpy = readfds;
	int nfds = MAX(fileno(stdin), ifacep->inputfd) + 1;
	int ret = select(nfds, &cpy, NULL, NULL, NULL);
	if(ret < 0) {
		interface_appendoutput(ifacep, "select failed.");
		return -1;
	} else if(ret > 0) {
		if(FD_ISSET(fileno(stdin), &cpy)) {
			int ch = fgetc(stdin);
			if(userinput(ifacep, machinep, ch))
				return -1;

		} else if(FD_ISSET(ifacep->inputfd, &cpy))
			cliread(ifacep, machinep);
	}

	return 0;
}

int cli(struct machine *machinep, bool executefirst)
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

	iface->inputfd = outfd[0];
	iface->outputfd = infd[1];

	// prehook procedure for brainfuck interpreter
	machinep->prehook = brainfuck_prehook;
	machinep->userptr = iface;

	iface->execute = true;		// execute brainfuck
	iface->mode = IM_DEFAULT;	// default mode

	//interface_appendoutput(iface, "posthook: %p\n", machinep->posthook);
	
	// enter raw mode
	raw(true, true);

	// execute first?
	if(executefirst)
		machine_run(machinep);

	// loop interface
	while(cliloop(iface, machinep) == 0)
		/* do nothing */ ;

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
