#include <stdarg.h>
#include <string.h>
#include <zpsk.h>
#include "irc_cmd.h"

int irc_privmsg(socket_t *sck, const char *to, const char *fmt, ...)
{
	char buf[770];	/* most irc servers max out at 512 anyway. */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, 770, fmt, ap);
	va_end(ap);

	return irc_send(sck, "PRIVMSG %s :%s\r\n", to, buf);
}

int irc_notice(socket_t *sck, const char *to, const char *fmt, ...)
{
	char buf[770];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, 770, fmt, ap);
	va_end(ap);

	return irc_send(sck, "NOTICE %s :%s\r\n", to, buf);
}

int irc_join(socket_t *sck, const char *channel, const char *key)
{
	return irc_send(sck, "JOIN %s %s\r\n", channel, (key? key: ""));
}

int irc_part(socket_t *sck, const char *channel, const char *pmsg)
{
	return irc_send(sck, "PART %s :%s\r\n", channel, pmsg);
}

int irc_mode(socket_t *sck, const char *channel, const char *mode, ...)
{
	va_list ap;
	char *p, buf[770];
	int i = 0;

	va_start(ap, mode);
	while((p = va_arg(ap, char*)) && i < 770)
		i += snprintf(&buf[i], 770 - i, "%s ", p);
	va_end(ap);

	return irc_send(sck, "MODE %s %s %s\r\n", channel, mode, buf);
}

int irc_quit(socket_t *sck, const char *qmsg)
{
	return irc_send(sck, "QUIT :%s\r\n", qmsg);
}

int irc_topic(socket_t *sck, const char *channel, const char *fmt, ...)
{
	va_list ap;
	char buf[770];

	va_start(ap, fmt);
	vsnprintf(buf, 770, fmt, ap);
	va_end(ap);

	return irc_send(sck, "TOPIC %s :%s\r\n", channel, buf);
}

int irc_kick(socket_t *sck, const char *channel, const char *who,
		const char *fmt, ...)
{
	va_list ap;
	char buf[770];

	va_start(ap, fmt);
	vsnprintf(buf, 770, fmt, ap);
	va_end(ap);

	return irc_send(sck, "KICK %s %s :%s\r\n", channel, who, buf);
}

int irc_send(socket_t *sck, const char *fmt, ...)
{
	char buf[1281];
	int q;
	va_list ap;

	va_start(ap, fmt);
	q = vsnprintf(buf, 1280, fmt, ap);
	va_end(ap);

	return sknputs(buf, sck, q);
}
