#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

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
				--mp->memp;
				break;

			case OPCODE_INCREMENT:
				++mp->memory[mp->memp];
				break;

			case OPCODE_DECREMENT:
				--mp->memory[mp->memp];
				break;

			case OPCODE_INPUT:
				mp->memory[mp->memp] = fgetc(stdin);
				break;

			case OPCODE_OUTPUT:
				fputc(mp->memory[mp->memp], stdout);
				fflush(stdout);
				break;

			default:
				fprintf(stderr, "Unknown opcode. 0x%02x\n",
						mp->ops[mp->ip].opcode);
		}
		++mp->ip;
	}
}

void raw(bool enable)
{
	static struct termios orig;
	struct termios new;

	if(enable) {
		tcgetattr(STDIN_FILENO, &orig);
		new = orig;
		new.c_lflag &= ~ICANON;
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &new);
	} else
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

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
	} else {
		long size = 256, length = 0;

		code = calloc(size, sizeof(char));
		if(!code) {
			fprintf(stderr, "cannot allocate buffer (%ld). out of memory?\n", size);
			return 1;
		}

		int ch;
		while((ch = fgetc(stdin)) != EOF) {
			code[length++] = ch;
			if(length >= size) {
				size <<= 1;	// double memory usage.

				char *newp = realloc(code, size);
				if(!newp) {
					perror("error");
					if(code) { free(code); }
					fprintf(stderr, "cannot reallocate buffer (%ld).\n", size);
					fflush(stderr);
					return 1;
				}

				if(code != newp)
					code = newp;
			}
		} // FIXME? potential boundry buffer overflow? (would be hard to execute though)
		code[length] = '\0';
	}

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

	raw(true);
	machine_run(machine);
	raw(false);

	machine_destroy(&machine);

	return 0;
}
