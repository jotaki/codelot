#ifndef H_IRC_CMD_
# define H_IRC_CMD_
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <zpsk.h>

# define MODE_OP            "+o"
# define MODE_DEOP          "-o"
# define MODE_VOICE         "+v"
# define MODE_DEVOICE       "-v"
# define MODE_BAN           "+b"
# define MODE_UNBAN         "-b"
# define MODE_EXEMPT        "+e"
# define MODE_UNEXEMPT      "-e"
# define MODE_MODERATE      "+m"
# define MODE_UNMODERATE    "-m"
# define MODE_MAXJOIN       "+l"
# define MODE_REMMAXJOIN    "-l"

int irc_privmsg(socket_t *sck, const char *to, const char *fmt, ...);
int irc_notice(socket_t *sck, const char *to, const char *fmt, ...);
int irc_join(socket_t *sck, const char *channel, const char *key);
int irc_part(socket_t *sck, const char *channel, const char *pmsg);
int irc_mode(socket_t *sck, const char *channel, const char *mode, ...);
int irc_quit(socket_t *sck, const char *qmsg);
int irc_topic(socket_t *sck, const char *channel, const char *fmt, ...);
int irc_kick(socket_t *sck, const char *channel, const char *who,
		const char *fmt, ...);

int irc_send(socket_t *sck, const char *fmt, ...);

#endif	/* H_IRC_CMD_ */

