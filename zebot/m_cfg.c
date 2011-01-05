#define __MODULE__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "zebot.h"

static char *getval(char **var, char *buf)
{
	char *p = buf, *t;
	int c;

	while((c = *p++)) {
		if(c == ' ') {
			for(t = p-1; *t != 0; t++)
				*t = t[1];
		}
	}
	t = strchr(buf, '=');
	*(t++) = 0;
	*var = buf;

	return t;
}

char *read_config(FILE *fp, const char *var, void *val)
{
	char *v1, *v2, *e, buf[768];

	while(fgets(buf, 768, fp)) {
		if((e = strchr(buf, '#'))) {
			if(e[-1] != '\\') *e = 0;
			else {
				for(v1 = e-1; *v1 != 0; v1++)
					*v1 = v1[1];
			}
		}

		if((e = strchr(buf, '\n'))) {
			if(e[-1] == '\r') e[-1] = 0;
			*e = 0;
		}

		if((e = strchr(buf, '='))) {
			v1 = getval(&v2, buf);
			if(!strcasecmp(v2, var)) {
				strcpy((char *)val, v1);
				break;
			}
		}
	}
	rewind(fp);
	return ((char*)val);
}

#undef __MODULE__
