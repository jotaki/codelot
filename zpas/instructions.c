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
#include <ctype.h>

#include "zpas.h"

#define inuse(u1, u2)	(u1 != NULL || u2 != NULL)

global_t *global;

char *registers[] = {
	"al", "ah", "ax", "eax",
	"bl", "bh", "bx", "ebx",
	"cl", "ch", "cx", "ecx",
	"dl", "dh", "dx", "edx",
};

/* ascii adjust after addition */
int i_aaa(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0x37, fp);
	global->byte++;
	return 0;
}

/* ascii adjust AX before devision */
int i_aad(FILE *fp, char *imm, char *uu)
{
	long ib = 0x0a;

	if(uu) return E_SYNTAX;
	else if(imm) ib = convert(imm);

	if((ib >> 8) & 0xff)
		return E_OOB;

	fputc(0xd5, fp);
	fputc(ib, fp);

	global->byte += 2;
	return 0;
}

/* ascii adjust AX after multiply */
int i_aam(FILE *fp, char *imm, char *uu)
{
	long ib = 0x0a;

	if(uu) return E_SYNTAX;
	else if(imm) ib = convert(imm);

	if((ib >> 8) & 0xff)
		return E_OOB;

	fputc(0xd4, fp);
	fputc(ib, fp);

	global->byte += 2;
	return 0;
}

/* ascii adjust al after subtraction */
int i_aas(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0x3f, fp);
	global->byte++;
	return 0;
}

/* clear carry flag */
int i_clc(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0xf8, fp);
	global->byte++;
	return 0;
}

/* clear direction flag */
int i_cld(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0xfc, fp);
	global->byte++;
	return 0;
}

/* clear interrupt flag */
int i_cli(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0xfa, fp);
	global->byte++;
	return 0;
}

/* clear task switched flag */
int i_clts(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0x0f, fp);
	fputc(0x06, fp);
	global->byte += 2;

	return 0;
}

/* complement carry flag */
int i_cmc(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0xf5, fp);
	global->byte++;
	return 0;
}

/* CPU identification */
int i_cpuid(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0x0f, fp);
	fputc(0xa2, fp);
	global->byte += 2;
	return 0;
}

/* halt */
int i_hlt(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fputc(0xf4, fp);
	global->byte++;
	return 0;
}

/* TODO: support all mov opcodes */

int i_mov(FILE *fp, char *dst, char *src)
{
	int i, i2 = strlen(dst) - 1;

	dprintf("mov %s, %s\n", dst, src);

	for(i = 0; i < sizeof_ary(registers); i++) {
		if(strcasecmp(src, registers[i]) == 0) {
			return E_OPCODESUP;
		}
	}

	i = tolower(dst[i2]);
	if(i == 'x' || i == 'p' || i == 'i') {
		if(tolower(dst[0]) != 'e')
			return (i_movw(fp, dst, src));
		else
			return (i_movd(fp, dst, src));
	} else
		return (i_movb(fp, dst, src));

	return E_SYNTAX;
}

/* move byte */
int i_movb(FILE *fp, char *dst, char *src)
{
	int b = (int) convert(src);

	if((b >> 8) & 0xff)
		return E_OOB;

	fputc(0xb0 + index_of_R(dst, 8), fp);
	fputc(b, fp);

	global->byte += 2;

	return 0;
}

/* move word */
int i_movw(FILE *fp, char *dst, char *src)
{
	long b = convert(src);

	fputc(0xb8 + index_of_R(dst, 16), fp);
	fputc(b & 0x00ff, fp);
	fputc((b >> 8) & 0xff, fp);

	global->byte += 3;

	return 0;
}

/* mov dword */
int i_movd(FILE *fp, char *dst, char *src)
{
	long b = convert(src);

	fputc(0x66, fp);	/* prefix 32-Bit */
	fputc(0xb8 + index_of_R(dst, 32), fp);
	fputc(b & 0xff, fp);
	fputc((b >> 8) & 0xff, fp);
	fputc((b >> 16) & 0xff, fp);
	fputc((b >> 24) & 0xff, fp);

	global->byte += 6;

	return 0;
}

int i_int(FILE *fp, char *irupt, char *uu)
{
	long intr = convert(irupt);

	if(inuse(uu, NULL))
		return E_SYNTAX;

	fprintf(fp, "\xCD%c", (int) intr);
	global->byte += 2;

	return 0;
}

int i_int3(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fprintf(fp, "\xCC");
	global->byte++;

	return 0;
}

int i_into(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fprintf(fp, "\xCE");
	global->byte++;
	return 0;
}

int i_nop(FILE *fp, char *uu1, char *uu2)
{
	if(inuse(uu1, uu2))
		return E_SYNTAX;

	fprintf(fp, "\x90");
	global->byte++;
	return 0;
}
