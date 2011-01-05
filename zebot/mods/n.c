#define __MODULE__

#include <zpsk.h>
#include "zebot.h"

#include <string.h>
#include <strings.h>
#include <mysql.h>

/* variable declarations */
struct idents {
	char nick[32];
	int level, identified;
	struct idents *next;
} *mdu_head = NULL;

struct mod_config {
	char nspass[64];
	char autojoin[512];
	char mysql_host[256];
	char mysql_user[64];
	char mysql_pass[64];
} md_cfg;

/* function declarations */
void on_privmsg();
void on_numeric();
void command();
void release_idents(void);
struct idents *find_ident();
struct idents *identify();

/* init/fini */
int default_module_init(void);
void default_module_fini(void);

/* our commands */
void on_privmsg(sck, from, to, msg)
	socket_t *sck;
	struct userinfo *from;
	const char *to, *msg;
{
	struct idents *u = find_ident(from->nick);

	printf("<%s/%s> %s\n", from->nick, to, msg);

	if(u && msg[0] == '.')
		command(sck, from, to, &msg[1]);
	else if(!u && !strcasecmp(msg, "\001ACTION identifies\001")) {
		printf("HIT\n");
		irc_send(sck, "WHOIS %s", from->nick);
	}
}

void on_numeric(sck, numeric, line)
	socket_t *sck;
	int numeric;
	const char *line;
{
	switch(numeric) {
		case 376:
		case 422:
			if(md_cfg.nspass[0] != 0)
				irc_privmsg(sck, "NickServ", "IDENTIFY %s", md_cfg.nspass);
			if(md_cfg.autojoin[0] != 0)
				irc_join(sck, md_cfg.autojoin, "");
		break;

		case 320: {
			char *nick = strchr(line, ' ');
			*(nick++) = 0;
			if(identify(nick))
				irc_notice(sck, nick, "You are now identified to ZeBot.");
		} break;
	}
}

/* some helper functions */
void command(sck, from, to, msg)
	socket_t *sck;
	struct userinfo *from;
	const char *to, *msg;
{
	int len = strlen(msg);

	if(!strncasecmp(msg, "join", 4) && len > 5)
		irc_join(sck, &msg[5], NULL);
	else if(!strncasecmp(msg, "part", 4)) {
		if(len > 5) irc_part(sck, &msg[5], "");
		else irc_part(sck, to, "");
	}
}

void release_idents(void)
{
	struct idents *ptr, *sptr;

	if(!mdu_head)
		return;

	for(ptr = mdu_head; ptr != NULL;) {
		sptr = ptr->next;
		free(sptr);
		ptr = sptr;
	}
}

struct idents *find_ident(nick)
	const char *nick;
{
	struct idents *p;

	for(p = mdu_head; p != NULL; p = p->next)
		if(!strcasecmp(p->nick, nick) && p->identified)
			return p;

	return NULL;
}

struct idents *identify(nick)
	const char *nick;
{
	struct idents *ptr;

	for(ptr = mdu_head; ptr != NULL; ptr = ptr->next) {
		if(!strcasecmp(ptr->nick, nick)) {
			ptr->identified = 1;
			return ptr;
		}
	}
	return NULL;
}

/* module ptr */
struct module_ptr mod_info_zb = {
	.mod_name = "Default",
	.mod_version = 0.0,
	.author = "Joseph K.",

	.init = default_module_init,
	.on_numeric = on_numeric,
	.on_privmsg = on_privmsg,
	.on_notice = NULL,
	.on_join = NULL,
	.on_part = NULL,
	.on_quit = NULL,
	.on_kick = NULL,
	.on_mode = NULL,
	.fini = default_module_fini
};

/* init/fini */
int default_module_init(void)
{
	MYSQL zbdb;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[1024];
	struct idents *new, *ptr;

	mysql_init(&zbdb);
	memset(&md_cfg, 0, sizeof(md_cfg));

	FILE *fp = fopen("zebot.cfg", "r");
	if(!fp) return 1;

	load_config(fp, &md_cfg, nspass);
	load_config(fp, &md_cfg, autojoin);
	load_config(fp, &md_cfg, mysql_host);
	load_config(fp, &md_cfg, mysql_user);
	load_config(fp, &md_cfg, mysql_pass);

	fclose(fp);

	if(!mysql_real_connect(&zbdb, md_cfg.mysql_host, md_cfg.mysql_user,
			md_cfg.mysql_pass, "zebot", 0, NULL, 0)) {
		return 1;
	}

	sprintf(query, "SELECT nick,level FROM users");
	if(mysql_real_query(&zbdb, query, strlen(query))) {
		mysql_close(&zbdb);
		return 1;
	}

	res = mysql_use_result(&zbdb);
	while((row = mysql_fetch_row(res))) {
		new = (struct idents *) calloc(sizeof(struct idents), 1);
		if(!new) {
			mysql_free_result(res);
			mysql_close(&zbdb);
			release_idents();
			return 1;
		}

		strncpy(new->nick, row[0], sizeof(new->nick));
		new->level = atoi(row[1]);
		new->identified = 0;
		new->next = NULL;

		if(!mdu_head)
			mdu_head = new;
		else {
			for(ptr = mdu_head; ptr->next != NULL; ptr = ptr->next);
			ptr->next = new;
		}
	}

	mysql_free_result(res);
	mysql_close(&zbdb);
	return 0;
}

void default_module_fini(void)
{
	release_idents();
}

#undef __MODULE__
