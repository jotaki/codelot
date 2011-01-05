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

#ifndef H_ZPAS_
# define H_ZPAS_

# define VERSION "0.1"
# define sizeof_ary(x)	(sizeof(x)/sizeof(x[0]))

# include <stdio.h>

typedef struct instructs_t {
	char *ins;
	int (*parse)();
} instructs_t;

typedef struct global_t {
	FILE *fin, *fout; /* File in, File out */
	long line, byte;  /* current line, current byte */
	char *output;     /* output filename */
} global_t;

enum {
	E_SYNTAX = 0x01,  /* Syntax error */
	E_UNKNOWNDEF,     /* Uknown defination */
	E_OUTOFMEM,       /* Out of Memory */
	E_OPCODESUP,      /* Opcode not supported yet */
	E_OOB,            /* Out of bounds (invalid operand for opcode) */
};

extern global_t *global;

void Error(int err);

char *strip(char *buf);
char *getSrc(char *buf);
char *getDest(char *buf);

int isdigits(char *str);
int index_of_R(char *reg, short bit);

long addr_of(char *rm);

long tolong(char *nptr, int base);

long convert_b(char *val);
long convert_h(char *val);
long convert_o(char *val);
long convert_d(char *val);
long convert(char *val);

int dprintf(char *fmt, ...);

int i_aaa(FILE *fp, char *uu1, char *uu2);
int i_aad(FILE *fp, char *imm, char *uu);
int i_aam(FILE *fp, char *imm, char *uu);
int i_aas(FILE *fp, char *uu1, char *uu2);

int i_clc(FILE *fp, char *uu1, char *uu2);
int i_cld(FILE *fp, char *uu1, char *uu2);
int i_cli(FILE *fp, char *uu1, char *uu2);
int i_clts(FILE *fp, char *uu1, char *uu2);
int i_cmc(FILE *fp, char *uu1, char *uu2);
int i_cpuid(FILE *fp, char *uu1, char *uu2);

int i_hlt(FILE *fp, char *uu1, char *uu2);

int i_mov(FILE *fp, char *src, char *dst);
int i_movb(FILE *fp, char *str, char *dst);
int i_movw(FILE *fp, char *str, char *dst);
int i_movd(FILE *fp, char *str, char *dst);

int i_int(FILE *fp, char *irupt, char *uu);
int i_int3(FILE *fp, char *uu1, char *uu2);
int i_into(FILE *fp, char *uu1, char *uu2);

int i_nop(FILE *fp, char *uu1, char *uu2);

int zpi_define(FILE *fp, char *key, char *uu);
int zpi_undef(FILE *fp, char *key, char *uu);

#endif
