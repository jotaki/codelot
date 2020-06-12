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
			for(int rep = 0; rep < repeat; ++rep)
				brainfuck_eval_chr(machinep, '>', true);

			break;

		// move left
		case '<':
		case 'h':
			for(int rep = 0; rep < repeat; ++rep)
				brainfuck_eval_chr(machinep, '<', true);

			break;

		// increment and decrement
		case '+':
		case '-':
			for(int rep = 0; rep < repeat; ++rep)
				brainfuck_eval_chr(machinep, ch, true);

			break;

		// move right by 16
		case 'j':
			for(int rep = 0; rep < repeat; ++rep) {
				for(int i = 0; i < FIXED_MEM_WIDTH; ++i)
					brainfuck_eval_chr(machinep, '>', true);
			}
			break;

		// move left by 16
		case 'k':
			for(int rep = 0; rep < repeat; ++rep) {
				for(int i = 0; i < FIXED_MEM_WIDTH; ++i)
					brainfuck_eval_chr(machinep, '<', true);
			}
			break;

		// move to beginning of 16 byte boundry
		case '^':
		{
			int boundry = machinep->memp;
			boundry %= FIXED_MEM_WIDTH;

			for(int i = 0; i < boundry; ++i)
				brainfuck_eval_chr(machinep, '<', true);
		}
		break;

		// move to end of 16 byte boundry
		case '$':
		{
			int boundry = machinep->memp;
			boundry %= FIXED_MEM_WIDTH;
			boundry = FIXED_MEM_WIDTH - boundry - 1;

			for(int i = 0; i < boundry; ++i)
				brainfuck_eval_chr(machinep, '>', true);
		}
		break;

		// shorthand for [-]
		case '_':
			brainfuck_compile(machinep, "[-]");
			machine_run(machinep);
			break;

		// putc
		case '.':
			brainfuck_eval_chr(machinep, ch, true);
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

int cli(struct machine *machinep)
{
	struct interface *iface = NULL;
#if 0
	struct winsize ws;
	int mp = 0, rep = 0, length = 0, outlength = 0, codelen = 0;
	int y = 2, x = 11;
	char line[2048], output[8192], code[CODE_SIZE];
	int infd[2], outfd[2], loopcnt = 0;
	struct { int startaddr, endaddr; } loop[0x1000] = { {0} };
#endif
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
#if 0

	if(pipe(infd) < 0) {
		perror("failed to create pipe");
		interface_destroy(&iface);
		return 1;
	}

	if(pipe(outfd) < 0) {
		perror("failed to create pipe");

		close(infd[0]);
		close(infd[1]);
	}

	if(!machine_create(&machine)) {
		close(infd[0]);
		close(infd[1]);
		close(outfd[0]);
		close(outfd[1]);

		return 1;
	}

	machine->input = infd[0];
	machine->output = outfd[1];

	raw(true, true);

	while(true) {
		termsize(&ws);
		clr();
		moveto(0, 0);

		printf("\033[7m\033[1m");
		printf("ip: %08x", machine->ip);

		for(int i = 0; i < 20; ++i) printf(" ");
		if(codelen > 10) {
			for(int i = codelen - 10; i < codelen; ++i)
				printf("%c", code[i]);
		} else {
			int i;

			for(i = 0; i < codelen; ++i)
				printf("%c", code[i]);

			for(; i < 10; ++i)
				printf(" ");
		}
		for(int i = 0; i < 20; ++i) printf(" ");

		printf("memp: %08x\n", machine->memp);
		printf("\033[0m");

		int offset = mp;
		int maxrows = ws.ws_row / 2;
		for(int row = 1; row < maxrows; ++row) {
			printf("%08x ", offset);

			for(int i = 0; i < 8; ++i)
				printf(" %02x", machine->memory[offset+i] & 0xff);
			
			for(int i = 0; i < 8; ++i)
				printf(" %02x", machine->memory[offset+8+i] & 0xff);

			printf(" |");
			for(int i = 0; i < 16; ++i) {
				if(isprint(machine->memory[offset+i] & 0xff))
					printf("%c", machine->memory[offset+i] & 0xff);
				else
					printf(".");
			}
			printf("|\n");
			offset += 16;
		}

		printf("\033[7m\033[1m");
		for(int i = 0; i < 76; ++i) printf("—");
		printf("\033[0m");

		moveto(ws.ws_row, 0);
		printf("\033[7m\033[1m");
		printf("input buffer: %s", line);
		for(int i = 0; i < 76 - (14+length); ++i) printf(" ");
		printf("\033[0m");

		moveto(maxrows+2, 0);
		printf("%s", output);
		fflush(stdout);

		moveto(y, x);

		int ch = fgetc(stdin);
		if(ch == 'q')
			break;
		else if(isdigit(ch))
			rep = rep * 10 + (ch - '0');

		switch(ch) {
			case 'j': 
				if(!rep) rep = 1;

				for(int i = 0; i < rep; ++i) {
					if(++y > maxrows) {
						y = maxrows;
						mp += 16;
					}

					for(int j = 0; j < 16; ++j) {
						brainfuck_eval_chr(machine, '>', true);
						code[codelen++] = '>';
						code[codelen] = '\0';
					}
				}

				rep = 0;
				break;

			case 'k':
				if(!rep) rep = 1;

				for(int i = 0; i < rep; ++i) {
					if(--y < 2) {
						y = 2;
						if(mp > 0) mp -= 16;
						if(mp < 0) mp = 0;
					}
				
					for(int j = 0; j < 16; ++j) {
						brainfuck_eval_chr(machine, '<', true);
						code[codelen++] = '<';
						code[codelen] = '\0';
					}
				}

				rep = 0;
				break;

			case '+':
			case '-':
				if(!rep) rep = 1;

				for(int i = 0; i < rep; ++i) {
					brainfuck_eval_chr(machine, ch, true);
					code[codelen++] = ch;
					code[codelen] = '\0';
				}

				rep = 0;
				break;

			case '>':
			case 'l':
				if(!rep) rep = 1;

				for(int i = 0; i < rep; ++i) {
					brainfuck_eval_chr(machine, '>', true);

					code[codelen++] = '>';
					code[codelen] = '\0';

					x += 3;
					if(x == 34) x += 1;
					if(x > 56) {
						++y;
						if(y > maxrows) {
							y = maxrows;
							mp += 16;
						}
						x = 11;
					}
				}

				rep = 0;
				break;

			case '<':
			case 'h':
				if(!rep) rep = 1;
				for(int i = 0; i < rep; ++i) {
					brainfuck_eval_chr(machine, '<', true);
					code[codelen++] = '<';
					code[codelen] = '\0';

					x -= 3;
					if(x == 36) x -= 1;
					if(x < 11) {
						if(y > 2) {
							--y;
							x = 56;
						} else if(y == 2 && mp != 0) {
							mp -= 16;
							x = 56;
						} else
							x = 11;
					}
				}
				break;

			case ',': {
				if(!rep) rep = 1;

				moveto(ws.ws_row, 15+length);
				for(int i = 0; i < rep; ++i) {
					int tmp = fgetc(stdin);
					line[length++] = tmp;
					line[length] = '\0';

					if(tmp == '\n') {
						length = 0;
						line[length] = '\0';
					} else if(length > 64) {
						for(int j = 1; j <= length; ++j)
							line[j-1] = line[j];

						--length;
					}

					write(infd[1], &tmp, 1);
					brainfuck_eval_chr(machine, ch, true);

					code[codelen++] = ch;
					code[codelen] = '\0';
				}

				moveto(y, x);
				rep = 0;
			}
			break;

			case '.': {
				if(!rep) rep = 1;

				for(int i = 0; i < rep; ++i) {
					int tmp;

					brainfuck_eval_chr(machine, ch, true);
					code[codelen++] = ch;
					code[codelen] = '\0';

					read(outfd[0], &tmp, 1);
					output[outlength++] = tmp;
					output[outlength] = '\0';

					if(outlength % 76 == 0) {
						output[outlength++] = '\n';
						output[outlength] = '\0';
					}
				}

				rep = 0;
			} break;

			case '?':
				showhelp(&ws);
				break;

			case '[':
				code[codelen++] = '[';
				code[codelen] = '\0';

				// hack
				if(!machine->memory[machine->memp]) {
					++loopcnt;
					while(loopcnt != 0) {
						int tmp = fgetc(stdin);

						code[codelen++] = tmp;
						code[codelen] = '\0';

						if(tmp == ']') --loopcnt;
						else if(tmp == '[') ++loopcnt;
					}
					break;
				}
				brainfuck_eval_chr(machine, '[', false);
				loop[loopcnt].startaddr = machine->codesize - 1;
				loop[loopcnt].endaddr = 0;
				++loopcnt;
				++machine->ip;	// hack

				break;

			case ']':
				if(--loopcnt < 0) {
					loopcnt = 0;
					break;
				}

				int tmp = loop[loopcnt].startaddr;

				machine->ops[tmp].addr = machine->codesize;
				machine->ops[machine->codesize].addr = tmp;

				brainfuck_eval_chr(machine, ']', loopcnt == 0);
				code[codelen++] = ']';
				code[codelen] = '\0';

				break;
		}
	}

	close(infd[0]);
	close(infd[1]);
	close(outfd[0]);
	close(outfd[1]);

	machine_destroy(&machine);
	clr();
	moveto(0, 0);
	raw(false, true);

	return 0;
}
#endif
