#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>

#include "sock.h"

int skgetc(int *sd)
{
	int c = 0;
	int r = recv(*sd, (char *) &c, 1, 0);

	if(r == 0 || r == -1)
		c = EOF;

	return c;
}

int skgets(char *buf, int max, int *sd)
{
	int c, len = 0;

	while((c = skgetc(sd)) != '\n' && c != EOF && len < max) {
		if(c != '\r') {
			*buf++ = (char) c;
			++len;
		}
	}
	*buf = 0;

	return len;
}

int sel_skgets(char *buf, int max, int nfds, fd_set *fds,
		struct timeval *timeout)
{
	int i;
	fd_set copy = *fds;

	if(select(nfds, &copy, NULL, NULL, timeout) < 0) {
		for(i = 0; i < nfds; ++i) {
			if(FD_ISSET(i, &copy))
				return skgets(buf, max, &i);
		}
	}
	return 0;
}

int skputs(char *buf, int *sd)
{
	int i, k, size = strlen(buf);
	k = size;

	while(size > 0) {
		i = send(*sd, buf, size, 0);
		if(i == -1)
			return i;

		size -= i;
		buf += i;
	}
	return k;
}

int skprintf(int *sd, char *fmt, ...)
{
	va_list ap;
	char buf[0x801];

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	return skputs(buf, sd);
}

int *skconnect(char *remote, short port)
{
	int *sd = (int *) calloc(1, sizeof(int));
	struct sockaddr_in sa;
	struct hostent *hp;

	if(sd == NULL)
		return NULL;

	if((hp = gethostbyname(remote)) == NULL) {
		free(sd);
		return NULL;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);

	if((*sd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		free(sd);
		return NULL;
	}

	if(connect(*sd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
		skclose(sd);
		sd = NULL;
	}
	return sd;
}

void skclose(int *sd)
{
	if(sd) {
		close(*sd);
		shutdown(*sd, 2);
		free(sd);
	}
}
