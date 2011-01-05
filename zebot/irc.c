#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include "irc.h"
#include "crc32tbl.h"

int fill_irc(char *str, struct irc *irc)
{
	char *f, *c, *r, *t;

	bzero(irc, 0);

	if(!str) return 1;

	if(*str == ':') {
		c = strchr(str, ' ');
		if(!c) return 1;
		*(c++) = 0;
		f = str + 1;
	} else {
		c = str;
		f = NULL;
	}

	r = strchr(c, ' ');
	if(!r) return 1;

	*(r++) = 0;
	irc->msg.cmd = c;

	if(f) {
		t = strchr(f, '!');
		if(t) {
			*(t++) = 0;
			irc->nick = f;
			irc->user = t;
			irc->host = strchr(t, '@');
			if(irc->host) *(irc->host++) = 0;
		} else
			irc->host = f;
	}

	irc->type = crc32(c);

	if(!strcasecmp(c, "PRIVMSG") || !strcasecmp(c, "NOTICE")) {
		char *to, *msg;

		to = r;
		msg = strchr(to, ':');
		if(!msg) return 1;
		*(msg++) = 0;

		if(c[0] == 'P') {
			irc->msg.privmsg.to = to;
			irc->msg.privmsg.msg = msg;
		} else {
			irc->msg.notice.to = to;
			irc->msg.notice.msg = msg;
		}
		return 0;
	} else if(!strcasecmp(c, "NICK")) {
		irc->msg.nick.new = r;
		return 0;
	} else if(!strcasecmp(c, "PING")) {
		irc->msg.pp.server = r;
	} else if(!strcasecmp(c, "JOIN")) {
		irc->msg.join.channel = r;
	} else if(!strcasecmp(c, "PART")) {
		char *chan, *pmsg = NULL;

		chan = r;
		pmsg = strchr(chan, ':');
		if(pmsg) *(pmsg++) = 0;

		irc->msg.part.channel = chan;
		irc->msg.part.pmsg = pmsg;
	} else if(!strcasecmp(c, "TOPIC")) {
		char *chan, *top;

		chan = r;
		top = strchr(chan, ' ');
		if(top) { *(top++) = 0; top++; }

		irc->msg.topic.channel = chan;
		irc->msg.topic.topic = top;
	} else {
		irc->line = r;

		if(atoi(c)) {
			irc->type = TYP_NUMERIC;
			irc->msg.reply = atoi(c);
		}
	}
	return 0;
}

unsigned int crc32(char *s)
{
	unsigned int crc = ~0;
	int k, l;

	l = strlen(s);
	for(k = 0; k < l; k++)
		crc = ((crc) >> 8) ^ crc32table[(s[k]) ^ ((crc) & 0xff)];

	return ~crc;
}
