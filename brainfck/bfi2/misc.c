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
