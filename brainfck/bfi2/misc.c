#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bfi2.h"

char *calculate_offset(long n)
{
	char *buf;
	int i, op = '>';

	if(n < 0) {
		op = '<';
		n = -n;
	} else if(n == 0)
		return NULL;

	buf = calloc(n+1, sizeof(char));
	if(!buf)
		return NULL;

	for(i = 0; i < n; ++i)
		buf[i] = op;

	buf[i] = '\0';
	return buf;
}

char *calculate_inverse(char *input)
{
	char *buf;
	int len = strlen(input);

	buf = calloc(len+1, sizeof(char));
	if(!buf)
		return NULL;

	for(int i = 0; i < len; ++i) {
		int op = input[i];

		switch(op) {
			case '>': op = '<'; break;
			case '<': op = '>'; break;
			case '+': op = '-'; break;
			case '-': op = '+'; break;
		}

		buf[i] = op;
	}
	buf[len] = '\0';

	return buf;
}

void alert(struct interface *ifacep, const char *fmt, ...)
{
	interface_appendoutput(ifacep, fmt);
}

void calculateyx(struct interface *ifacep, int *yp, int *xp)
{
	int height = ifacep->ws.ws_row / 2 - 2;
	int offset = 0;
	int maxbytes = FIXED_LINE_WIDTH * height;
	int y = 0, x = 0;

	if(height < 3) {
		*yp = 0;
		*xp = 0;

		return;
	}

	if(ifacep->output.length > maxbytes)
		offset = ifacep->output.length - maxbytes;

	y = ifacep->ws.ws_row / 2 + 1;
	x = 1;

	int newlines = 0;
	for(int i = ifacep->output.length; i >= offset; --i) {
		if(ifacep->output.buf[i] == '\n') {
			if(++newlines == height+2) {
				offset = i + 1;
				break;
			}
		}
	}		

	for(int i = offset; i < ifacep->output.length; ++i, ++x) {
		if(ifacep->output.buf[i] == '\n') {
			y = MIN(y+1, ifacep->ws.ws_row - 1);
			x = 0;
		}
	}

	*yp = y;
	*xp = x;
}

