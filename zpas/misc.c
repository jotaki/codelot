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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "zpas.h"
#include "define.h"

global_t *global;

void Error(int err)
{
	int line = global->line;

	switch(err) {
		case E_SYNTAX:
			fprintf(stderr, "Syntax error on line %d\n", line);
		break;

		case E_UNKNOWNDEF:
			fprintf(stderr, "Uknown defination on line %d\n", line);
		break;

		case E_OUTOFMEM:
			fprintf(stderr, "Out of memory\n");
		break;

		case E_OOB:
			fprintf(stderr, "Out of bounds. line %d\n", line);
		break;
	}

	define_shutdown();
	fclose(global->fin);
	fclose(global->fout);
	free(global);
	unlink(global->output);
	exit(err);
}

char *strip(char *buf)
{
	char *p = buf;
	char *s;

	if((s = strchr(p, ';')) != NULL)
		*s = 0;

	if((s = strchr(p, '\n')) != NULL)
		*s = 0;

	while(*p == '\t' || *p == ' ') {
		for(s = p; *s; s++)
			*s = *(s+1);
	}

	while(*p++) {
		if(*p == '\t')
			*p = ' ';

		if(*p == ' ' && (*(p+1) == ' ' || *(p+1) == '\t')) {
			for(s = p; *s; s++)
				*s = *(s+1);
		}
	}
	return buf;
}

char *getDest(char *buf)
{
	char *dst = NULL;

	if(buf == NULL)
		return NULL;

	dst = strchr(buf, ' ');
	if(dst != NULL)
		*(dst++) = 0;

	return dst;
}

char *getSrc(char *buf)
{
	char *src = NULL;

	if(buf == NULL)
		return NULL;

	src = strchr(buf, ',');
	if(src != NULL) {
		*(src++) = 0;
		src++;
	}
	return src;
}

int isdigits(char *str)
{
	while(*str) {
		if(!isdigit(*str++))
			return 0;
	}
	return 1;
}

int index_of_R(char *reg, short bit)
{
	char *r8[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
	char *r16_32[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
	char *cmp = reg;
	int i;

	if(bit == 8) {
		for(i = 0; i < 8; i++) {
			if(strcasecmp(cmp, r8[i]) == 0)
				return i;
		}
	} else if(bit == 16 || bit == 32) {
		if(bit == 32)
			cmp++;

		for(i = 0; i < 8; i++) {
			if(strcasecmp(cmp, r16_32[i]) == 0)
				return i;
		}
	}
	return 0;
}

/* TODO: actually make this function do it's stuff */
long addr_of(char *rm)
{
	return 0;
}

long tolong(char *nptr, int base)
{
	char *eptr[1];
	long l = strtoll(nptr, eptr, base);

	if(strcmp(nptr, eptr[0]) == 0)
			Error(E_SYNTAX);
	return l;
}

long convert_b(char *val)
{
	char *p = val;
	long r = 0, ei = strlen(val)-1;

	if(p[ei] == 'b')
		p[ei] = 0;
	else
		p += 2;

	for(; *p; p++) {
		r *= 2;
		if(*p == '1') r++;
		else if(*p != '0') {
			printf("Invalid binary number: %s\n", val);
			Error(E_SYNTAX);
		}
	}
	return r;
}

long convert_h(char *val)
{
	char *p = val;
	int ei = strlen(val)-1;
	
	if(p[ei] == 'h')
		p[ei] = 0;
	else
		p += 2;

	return tolong(p, 16);
}

long convert_o(char *val)
{
	char *p = val;
	int ei = strlen(val)-1;

	if(p[ei] == 'o')
		p[ei] = 0;

	return tolong(p, 8);
}

long convert_d(char *val)
{
	int ei = strlen(val) - 1;

	if(val[ei] == 'd')
		val[ei] = 0;

	return tolong(val, 10);
}

long convert(char *val)
{
	int ei = strlen(val) - 1;

	if((val[ei] == 'b') || (val[0] == '0' && val[1] == 'y'))
		return convert_b(val);
	else if((val[ei] == 'h') || (val[0] == '0' && val[1] == 'x'))
		return convert_h(val);
	else if((val[ei] == 'o') || (val[0] == '0'))
		return convert_o(val);
	else if((val[ei] == 'd') || (isdigits(val)))
			return convert_d(val);
	else {
		if(define_lookup(val) != NULL)
			return convert(define_lookup(val));
		else
			Error(E_UNKNOWNDEF);
	}
	Error(E_SYNTAX);
	return 0;
}

#ifdef DEBUG
int dprintf(char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vfprintf(stdout, fmt, ap);
	va_end(ap);

	return r;
}
#else
int dprintf(char *fmt, ...)
{
	return 0;
}
#endif
