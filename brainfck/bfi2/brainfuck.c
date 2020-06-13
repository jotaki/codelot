#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void brainfuck_funcmove(struct interface *ifacep, struct machine *mp)
{
	(void) ifacep;
	(void) mp;
}
