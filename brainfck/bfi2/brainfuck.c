#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#include "bfi2.h"

int brainfuck_compile(struct machine *mp, const char *code)
{
	unsigned int index = mp->codesize;

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

				mp->ops[index].param.addr = index + addr + 1;
				++index;
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

				mp->ops[index].param.addr = index - addr;
				++index;
			}
			break;
		}
	}

	mp->codesize = index;
	return 0;
}

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

	if(execute) machine_run(mp);
	else        machine_skip(mp);
}

int brainfuck_prehook(struct machine *machinep, enum opcode opcode, void *data)
{
	struct interface *ifacep = (struct interface *) data;

	if(opcode == OPCODE_INPUT && ifacep->mode != IM_INPUT) {
		enum interface_mode oldmode = ifacep->mode;

		ifacep->mode = IM_INPUT;

		termsize(&ifacep->ws);
		clr();
		moveto(0,0);

		drawheader(ifacep, machinep);
		drawseparator(ifacep);
		drawfooter(ifacep, machinep);

		updatestartaddr(ifacep, machinep);

		showmemory(ifacep, machinep);
		showoutput(ifacep, machinep);
		showcode(ifacep, machinep);

		updatecursor(ifacep, machinep);

		int ch = fgetc(stdin);
		interface_appendoutput(ifacep, "%c", ch);
		if(write(ifacep->outputfd, &ch, 1) < 0) {
			interface_appendoutput(ifacep, "\nwrite failed.\n");
			return -1;
		}

		ifacep->mode = oldmode;
	}
	return 0;
}

void brainfuck_posthook(struct machine *machinep, enum opcode opcode, void *data)
{
#if 0
	(void) machinep;
	(void) opcode;
	(void) data;
#else
	struct interface *ifacep = (struct interface *) data;

	if(opcode == OPCODE_OUTPUT) {
		struct timeval tv = { .tv_sec = 0, .tv_usec = 250 };
		fd_set fds;

		FD_ZERO(&fds);
		FD_SET(ifacep->inputfd, &fds);
	
		if(select(ifacep->inputfd+1, &fds, NULL, NULL, &tv) > 0)
			cliread(ifacep, machinep);
	}
#endif
}
