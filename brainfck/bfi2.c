#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>

#ifndef MIN
# define MIN(a,b)	((a)<(b)?(a):(b))
#endif


enum opcode {
	OPCODE_TRAP = 0x00,
	OPCODE_JUMP,
	OPCODE_JUMPZERO,
	OPCODE_MOVERIGHT,
	OPCODE_MOVELEFT,
	OPCODE_INCREMENT,
	OPCODE_DECREMENT,
	OPCODE_INPUT,
	OPCODE_OUTPUT,
};

#define MEMORY_SIZE	0x10000		// 64K
#define CODE_SIZE	0x10000

struct opcodes {
	enum opcode opcode;
	unsigned int addr;
};

struct machine {
	char memory[MEMORY_SIZE];
	struct opcodes ops[CODE_SIZE];

	unsigned int ip;	// instruction pointer.
	unsigned int memp;	// memory pointer.
	unsigned int codesize;	// code size.

	int input, output;
};

int brainfuck_compile(struct machine *mp, const char *code)
{
	unsigned int index = 0;

	for(const char *instruction = code; *instruction; instruction++) {
		switch(*instruction) {
			case '+':
				mp->ops[index++].opcode = OPCODE_INCREMENT;
				break;

			case '-':
				mp->ops[index++].opcode = OPCODE_DECREMENT;
				break;

			case '<':
				mp->ops[index++].opcode = OPCODE_MOVELEFT;
				break;

			case '>':
				mp->ops[index++].opcode = OPCODE_MOVERIGHT;
				break;

			case ',':
				mp->ops[index++].opcode = OPCODE_INPUT;
				break;

			case '.':
				mp->ops[index++].opcode = OPCODE_OUTPUT;
				break;

			case '[': {
				const char *p = instruction;
				unsigned short hits = 0;
				unsigned int addr = -1;

				mp->ops[index].opcode = OPCODE_JUMPZERO;
				for(p = instruction; *p; ++p) {
					if(*p == '[') ++hits;
					else if(*p == ']') --hits;
					if(hits == 0) break;

					if(strchr("[<>+-,.]", *p))
						++addr;
				}

				if(!*p || hits > 0)
					return -1;

				mp->ops[index++].addr = index + addr;
			}
			break;
				
			case ']': {
				const char *p = instruction;
				unsigned short hits = 0;
				unsigned int addr = 0;

				mp->ops[index].opcode = OPCODE_JUMP;
				for(p = instruction; p >= code; --p) {
					if(*p == ']') ++hits;
					else if(*p == '[') --hits;
					if(hits == 0) break;

					if(strchr("[<>+-,.]", *p))
						++addr;
 
				}
				
				if(p < code || hits > 0)
					return -1;

				mp->ops[index].addr = index - addr;
				++index;
			}
			break;
		}
	}

	mp->codesize = index;
	return 0;
}

struct machine *machine_create(struct machine **machinep)
{
	struct machine *newmachine = calloc(1, sizeof(struct machine));

	if(machinep)
		*machinep = newmachine;

	if(newmachine) {
		newmachine->input = fileno(stdin);
		newmachine->output = fileno(stdout);
	}

	return newmachine;
}

void machine_destroy(struct machine **machinep)
{
	if(machinep  && *machinep) {
		free(*machinep);
		*machinep = NULL;
	}
}

void machine_run(struct machine *mp)
{
	while(mp->ip < mp->codesize) {
		switch(mp->ops[mp->ip].opcode) {
			case OPCODE_TRAP:
				fprintf(stderr, "Reached unknown code space.\n");
				fflush(stderr);
				break;

			case OPCODE_JUMP:
				mp->ip = mp->ops[mp->ip].addr - 1;
				break;

			case OPCODE_JUMPZERO:
				if(mp->memory[mp->memp] == 0)
					mp->ip = mp->ops[mp->ip].addr;
				break;

			case OPCODE_MOVERIGHT:
				mp->memp = MIN(mp->memp+1, MEMORY_SIZE);
				break;

			case OPCODE_MOVELEFT:
				if((int) --mp->memp < 0)
					mp->memp = 0;

				break;

			case OPCODE_INCREMENT:
				++mp->memory[mp->memp];
				break;

			case OPCODE_DECREMENT:
				--mp->memory[mp->memp];
				break;

			case OPCODE_INPUT:
				read(mp->input, &mp->memory[mp->memp], 1);
				break;

			case OPCODE_OUTPUT:
				write(mp->output, &mp->memory[mp->memp], 1);
				break;

			default:
				fprintf(stderr, "Unknown opcode. 0x%02x\n",
						mp->ops[mp->ip].opcode);
		}
		++mp->ip;
	}
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

int cli(void);

int main(int argc, char *argv[])
{
	char *code = NULL;

	if(argc >= 2) {
		FILE *fp = fopen(argv[1], "r");
		if(!fp) {
			perror("error");
			fprintf(stderr, "failed to open ``%s''\n", argv[1]);
			return 1;
		}

		long size;

		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if(size <= 0) {
			fprintf(stderr, "empty file ``%s''\n", argv[1]);
			fclose(fp);
			return 0;
		}

		code = calloc(size+1, sizeof(char));
		if(!code) {
			perror("error");
			fprintf(stderr, "cannot allocate buffer (%ld). out of memory?\n", size);
			fclose(fp);
			return 1;
		}

		int length = 0, ch;
		while((ch = fgetc(fp)) != EOF)
			code[length++] = ch;

		code[length] = '\0';
		fclose(fp);
	} else
		return cli();

	// at this point, code shoud contain brainfuck code.
	struct machine *machine = machine_create(NULL);
	if(!machine) {
		fprintf(stderr, "could not create machine.\n");
		free(code);
		return 1;
	}


	if(brainfuck_compile(machine, code) < 0) {
		fprintf(stderr, "failed to compile code.\n");
		fflush(stdout);

		free(code);
		free(machine);
		return 1;
	}

	free(code);
	code = NULL;

	raw(true, false);
	machine_run(machine);
	raw(false, false);

	machine_destroy(&machine);

	return 0;
}

void clr() { printf("\033[2J"); fflush(stdout); }
void moveto(int y, int x) { printf("\033[%d;%dH",y,x); fflush(stdout); }

void brainfuck_eval_chr(struct machine *mp, int ch, bool execute)
{
	switch(ch) {
		case '+':
			mp->ops[mp->codesize++].opcode = OPCODE_INCREMENT;
			break;

		case '-':
			mp->ops[mp->codesize++].opcode = OPCODE_DECREMENT;
			break;

		case '<':
			mp->ops[mp->codesize++].opcode = OPCODE_MOVELEFT;
			break;

		case '>':
			mp->ops[mp->codesize++].opcode  = OPCODE_MOVERIGHT;
			break;

		case ',':
			mp->ops[mp->codesize++].opcode = OPCODE_INPUT;
			break;

		case '.':
			mp->ops[mp->codesize++].opcode = OPCODE_OUTPUT;
			break;

		case '[':
			mp->ops[mp->codesize++].opcode = OPCODE_JUMPZERO;
			break;

		case ']':
			mp->ops[mp->codesize++].opcode = OPCODE_JUMP;
			break;
	}

	if(execute)
		machine_run(mp);
}

int termsize(struct winsize *wsp)
{
	return ioctl(STDOUT_FILENO, TIOCGWINSZ, wsp);
}

void showhelp(struct winsize *ws)
{
	int y, x;

	clr();

	y = ws->ws_row / 4;
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

int cli(void)
{
	struct machine *machine = NULL;
	struct winsize ws;
	int mp = 0, rep = 0, length = 0, outlength = 0, codelen = 0;
	int y = 2, x = 11;
	char line[2048], output[8192], code[CODE_SIZE];
	int infd[2], outfd[2], loopcnt = 0;
	struct { int startaddr, endaddr; } loop[0x1000] = { {0} };

	if(pipe(infd) < 0) {
		perror("failed to create pipe");
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
