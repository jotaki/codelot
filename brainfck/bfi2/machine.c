#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bfi2.h"

struct machine *machine_create(struct machine **machinep)
{
	struct machine *newmachine = calloc(1, sizeof(struct machine));

	if(machinep)
		*machinep = newmachine;

	if(newmachine) {
		newmachine->inputfd = fileno(stdin);
		newmachine->outputfd = fileno(stdout);
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

char *machine_translate(enum opcode op)
{
	char *retbuf = "unknown";

	switch(op) {
		case OPCODE_TRAP: retbuf = "trap"; break;
		case OPCODE_JUMP: retbuf = "]"; break;
		case OPCODE_JUMPZERO: retbuf = "["; break;
		case OPCODE_MOVERIGHT: retbuf = ">"; break;
		case OPCODE_MOVELEFT: retbuf = "<"; break;
		case OPCODE_INCREMENT: retbuf = "+"; break;
		case OPCODE_DECREMENT: retbuf = "-"; break;
		case OPCODE_INPUT: retbuf = ","; break;
		case OPCODE_OUTPUT: retbuf = "."; break;
	}

	return retbuf;
}

void machine_run(struct machine *mp)
{
	while(mp->ip < mp->codesize) {
		/* fprintf(stderr, "%08x  %02x (%s)\n", mp->ip, mp->ops[mp->ip].opcode,
			machine_translate(mp->ops[mp->ip].opcode)); */

		switch(mp->ops[mp->ip].opcode) {
			case OPCODE_TRAP:
				fprintf(stderr, "Reached unknown code space.\n");
				fflush(stderr);
				break;

			case OPCODE_JUMP:
				mp->ip = mp->ops[mp->ip].param.addr - 1;
				break;

			case OPCODE_JUMPZERO:
				if(mp->memory[mp->memp] == 0)
					mp->ip = mp->ops[mp->ip].param.addr;
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
				if(read(mp->inputfd, &mp->memory[mp->memp], 1) < 0)
					perror("read failed");

				break;

			case OPCODE_OUTPUT:
				if(write(mp->outputfd, &mp->memory[mp->memp], 1) < 0)
					perror("write failed");

				break;

			default:
				fprintf(stderr, "Unknown opcode. 0x%02x\n",
						mp->ops[mp->ip].opcode);
		}
		++mp->ip;
	}
}

void machine_skip(struct machine *machinep)
{
	machinep->ip = machinep->codesize;
}
