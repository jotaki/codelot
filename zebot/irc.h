#ifndef H_IRC_
# define H_IRC_

# define TYP_PRIVMSG     0x45163144
# define TYP_NOTICE      0xb8f58244
# define TYP_JOIN        0x1ef43563
# define TYP_PART        0x7f9a9d72
# define TYP_NICK        0x1f9ec283
# define TYP_QUIT        0x753f3a21
# define TYP_NUMERIC     0x78e46251
# define TYP_PING        0x1340d049
# define TYP_TOPIC       0x6a141cab

struct userinfo
{
	char *nick;
	char *user;
	char *host;
};

struct irc
{
	char *nick;
	char *user;
	char *host;
	unsigned int type;

	struct {
		char *cmd;
		int reply;

		struct {
			char *to, *msg;
		} privmsg, notice;

		struct {
			char *channel, *pmsg;
		} join, part;

		struct {
			char *new;
		} nick;

		struct {
			char *server;
		} pp;

		struct topic {
			char *channel, *topic;
		} topic;
	} msg;

	char *line;
};

int fill_irc(char *str, struct irc *irc);

unsigned int crc32(char *s);

#endif	/* H_IRC_ */
