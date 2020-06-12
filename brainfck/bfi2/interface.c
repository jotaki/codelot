#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "bfi2.h"

struct interface *interface_create(struct interface **ifacep)
{
	struct interface *iface = calloc(1, sizeof(struct interface));

	if(ifacep)
		*ifacep = iface;

	return iface;
}

void interface_destroy(struct interface **ifacep)
{
	if(ifacep && *ifacep) {
		free(*ifacep);
		*ifacep = NULL;
	}
}

void interface_clroutput(struct interface *ifacep)
{
	ifacep->output.buf[0] = '\0';
	ifacep->output.length = 0;
}

void interface_appendoutput(struct interface *ifacep, const char *fmt, ...)
{
	va_list ap;
	int offset = ifacep->output.length;

	va_start(ap,fmt);
	offset += vsprintf(&ifacep->output.buf[offset], fmt, ap);
	va_end(ap);

	ifacep->output.buf[offset] = '\0';
	ifacep->output.length = offset;
}
