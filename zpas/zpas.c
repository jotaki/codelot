/*

    zpas - zer0python's simple binary assembler
    Copyright (C) 2005 Joseph Kinsella <zer0python@yahoo.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
	

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "zpas.h"
#include "define.h"

/* all the instructions go here */

instructs_t instructs[] = {
	{"aaa", &i_aaa},
	{"aad", &i_aad},
	{"aam", &i_aam},
	{"aas", &i_aas},

	{"clc", &i_clc},
	{"cld", &i_cld},
	{"cli", &i_cli},
	{"clts", &i_clts},
	{"cmc", &i_cmc},
	{"cpuid", &i_cpuid},

	{"hlt", &i_hlt},

	{"int", &i_int},
	{"int3", &i_int3},
	{"into", &i_into},

	{"mov", &i_mov},

	{"nop", &i_nop},

	{"#define", &zpi_define},
	{"#undef", &zpi_undef},
};

global_t *global;

int main(int argc, char *argv[])
{
	char iBuf[513];
	int i = 0;

	if(argc < 3) {
		printf("Usage: %s <input> <output>\n", argv[0]);
		return 0;
	}

	global = (global_t*) malloc(sizeof(global_t));
	if(global == NULL) {
		printf("out of memory");
		return 1;
	}

	memset(global, 0, sizeof(global));
	global->line = 1;
	global->byte = 0;
	global->output = argv[2];

	if(define_init()) {
		printf("out of memory\n");
		return 1;
	}

	global->fin = fopen(argv[1], "r");
	if(global->fin == NULL) {
		perror("could not open input file");
		return errno;
	}

	global->fout = fopen(argv[2], "wb");
	if(global->fout == NULL) {
		i = errno;

		perror("could open output file");
		fclose(global->fin);

		return i;
	}

	for(; fgets(iBuf, 512, global->fin) != NULL; global->line++) {
		if(iBuf[0] == ';' || iBuf[0] == '\n')
			continue;

		char *ins = strip(iBuf);
		char *dst = getDest(ins);
		char *src = getSrc(dst);
		int r = 0;
		
		for(i = 0; i < sizeof_ary(instructs); i++) {
			if(strcasecmp(instructs[i].ins, ins) == 0) {
				r = instructs[i].parse(global->fout, dst, src);
				if(r == 0)
					break;
				else
					Error(r);
			}
		}
		if(i == sizeof_ary(instructs))
			Error(E_SYNTAX);
	}

	define_shutdown();
	fclose(global->fin);
	fclose(global->fout);
	free(global);

	return 0;
}
