#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <zpsk.h>
#include <mysql.h>
#include <dlfcn.h>
#include "irc.h"
#include "irc_cmd.h"
#include "zebot.h"

int main(int argc, char *argv[])
{
	struct module_ptr *modhead;
	struct config cfg;
	socket_t *sck;
	char buf[2048];

	memset(&cfg, 0, sizeof(cfg));
	strcpy(cfg.filename, "zebot.cfg");

	if(argc != 1) { strncpy(cfg.filename, argv[1], 256); }
	if(load_config(&cfg)) {
		printf("Could not read config file: %s -- Exiting\n", cfg.filename);
		return 1;
	}

#ifdef EBUG
# define A(x) printf("[*] %s: %s\n", #x, cfg.x)
	A(filename);
	printf("------------------------------------\n");
	A(server);
	printf("[*] port: %hd\n", cfg.port);
	printf("------------------------------------\n");
	A(nickname);
	A(username);
	A(userinfo);
	printf("------------------------------------\n");
	A(ircpass);
	A(nspass);
	printf("------------------------------------\n");
# undef A

#endif	/* !EBUG */

	modhead = calloc(sizeof(struct module_ptr), 1);
	if(!modhead) {
		printf("Could not initialize module links\n");
		return 1;
	}

	modhead->mod_index = 0;
	if(!load_module(modhead, "./mods/default.so")) {
		free(modhead);
		printf("Could not load default module, not going to start with out this.\n");
		return 1;
	}

	sck = skconnect(cfg.server, cfg.port);
	if(!sck) {
		free(modhead);
		printf("could not connect to %s:%hu\n", cfg.server, cfg.port);
		return 1;
	}

	if(cfg.ircpass[0] != 0)
		skprintf(sck, "PASS %s\r\n", cfg.ircpass);

	skprintf(sck, "NICK %s\r\n", cfg.nickname);
	skprintf(sck, "USER %s * * :%s\r\n", cfg.username, cfg.userinfo);

	while(skgets(buf, 2048, sck)) {
#ifdef EBUG
		printf("%s\n", buf);
#endif

		run_modules(modhead, sck, buf, &cfg);
	}

	skclose(sck);
	cleanup_module(&modhead);
	return 0;
}

struct module_ptr *load_module(struct module_ptr *head, const char *path)
{
	struct module_ptr *info, *ptr, *new = calloc(sizeof(struct module_ptr), 1);

	if(!new) { return NULL; }

	new->handle = dlopen(path, RTLD_LAZY);
	if(!new->handle) {
		free(new);
		return NULL;
	}

	strncpy(new->mod_pf, path, sizeof(new->mod_pf));

	info = (struct module_ptr *) dlsym(new->handle, "mod_info_zb");
	if(info) {
		strcpy(new->mod_name, info->mod_name);
		strcpy(new->author, info->author);
		new->mod_version = info->mod_version;

#define A(x) new->x = info->x
		A(init);
		A(on_numeric);
		A(on_privmsg);
		A(on_notice);
		A(on_join);
		A(on_quit);
		A(on_kick);
		A(on_mode);
		A(fini);
#undef A

		if((unsigned long)new->init == 0)
			new->init = fake_init;

		if((unsigned long)new->fini == 0)
			new->fini = fake_fini;

#ifdef EBUG
		printf("[*] Module Name: %s\n", info->mod_name);
		printf("[*] Module Version: %f\n", info->mod_version);
		printf("[*] Author: %s\n", info->author);
		printf("-------------------------------------\n");
#define PFUNC(x) printf("[*] %s: %p\n", #x, info->x)
		PFUNC(init);
		PFUNC(on_numeric);
		PFUNC(on_privmsg);
		PFUNC(on_notice);
		PFUNC(on_join);
		PFUNC(on_part);
		PFUNC(on_quit);
		PFUNC(on_kick);
		PFUNC(on_mode);
		PFUNC(fini);
#undef PFUNC
		printf("-------------------------------------\n");
#endif	/* EBUG */
	}

	if(new->init()) {
		dlclose(new->handle);
		free(new);
		return NULL;
	}

	ptr = head;
	while(ptr->next != NULL) { ptr = ptr->next; }
	new->mod_index = ptr->mod_index + 1;

	return (ptr->next = new);
}

void unload_module(struct module_ptr *head, short index, char *path)
{
	struct module_ptr *p, *mp;

	if(!index) return;

	for(p = head; p->next != NULL; p = p->next) {
		if(p->next->mod_index == index) {
			p->next->fini();
			mp = p->next->next;
			strncpy(path, p->next->mod_pf, 512);
			free(p->next);
			p->next = mp;
			break;
		}
	}
}

void cleanup_module(struct module_ptr **head)
{
	struct module_ptr *ptr, *sptr;

	for(ptr = (*head)->next; ptr != NULL;) {
		sptr = ptr->next;
		ptr->fini();
		dlclose(ptr->handle);
		free(ptr);
		ptr = sptr;
	}
	free(*head);
}

#define RUN_M(f, p) \
	for(mp = head->next; mp != NULL; mp = mp->next) { \
		if((unsigned long) mp->f != 0) { \
			strncpy(mp->local, buf, sizeof(mp->local)); \
			fill_irc(mp->local, &mp->irc); \
			ui.nick = mp->irc.nick; ui.user = mp->irc.user; ui.host = mp->irc.host; \
			mp->f p; \
		} \
	}

void run_modules(struct module_ptr *head, socket_t *sck, const char *buf,
		struct config *cfg)
{
	struct irc *irc = &head->irc;
	struct module_ptr *mp;
	struct userinfo ui = { irc->nick, irc->user, irc->host };

	strncpy(head->local, buf, sizeof(head->local));
	fill_irc(head->local, irc);

	switch(irc->type) {
		case TYP_PRIVMSG:
			if(!strncasecmp(irc->msg.privmsg.msg, cfg->nickname,
					strlen(cfg->nickname))) {
				char *s = irc->msg.privmsg.msg + strlen(cfg->nickname);
				if(!strncasecmp(s, ": mc-", 5) && strlen(s)>5) {
					if(irc->msg.privmsg.to[0] == '#')
						mod_cmd(head, sck, irc->msg.privmsg.to, &s[5]);
					else
						mod_cmd(head, sck, ui.nick, &s[5]);
				}
			}
			RUN_M(on_privmsg, (sck, &ui, irc->msg.privmsg.to, irc->msg.privmsg.msg));
		break;

		case TYP_NUMERIC:
			RUN_M(on_numeric, (sck, irc->msg.reply, irc->line));
		break;

		case TYP_NOTICE:
			RUN_M(on_notice, (sck, &ui, irc->msg.notice.to, irc->msg.notice.msg));
		break;

		case TYP_JOIN:
			RUN_M(on_join, (sck, &ui, irc->msg.join.channel));
		break;

		case TYP_PART:
			RUN_M(on_part, (sck, &ui, irc->msg.part.channel, irc->msg.part.pmsg));
		break;

		case TYP_PING:
			skprintf(sck, "PONG %s\r\n", irc->msg.pp.server);
		break;
	}
}
#undef RUN_M

void mod_cmd(struct module_ptr *head, socket_t *sck,
		const char *rpl, const char *cmd)
{
	struct module_ptr *p;
	char tmp[512];

	if(!strcasecmp(cmd, "list")) {
		for(p = head->next; p != NULL; p = p->next) {
			irc_privmsg(sck, rpl, "Module #%u: '%s' version %2.2f written by: %s",
				p->mod_index, p->mod_name, p->mod_version, p->author);
		}
	}	else if(!strncasecmp(cmd, "reload", 6) && strlen(cmd) > 7) {
		if(atoi(&cmd[7])) {
			unload_module(head, atoi(&cmd[7]), tmp);
			p = load_module(head, tmp);
			irc_privmsg(sck, rpl, "Module '%s %2.2f' now reloaded",
				p->mod_name, p->mod_version);
		}
	} else if(!strncasecmp(cmd, "load", 4) && strlen(cmd) > 5) {
		if((p = load_module(head, &cmd[5]))) {
			irc_privmsg(sck, rpl, "Module '%s %2.2f' is now loaded",
				p->mod_name, p->mod_version);
		}
	} else if(!strncasecmp(cmd, "unload", 6) && strlen(cmd) > 7) {
		unload_module(head, atoi(&cmd[7]), tmp);
	}
}

int load_config(struct config *cfg)
{
	FILE *fp = fopen(cfg->filename, "r");
	char *var, *val, *e, buf[768];

	if(!fp) { return -1; }

	while(fgets(buf, sizeof(buf), fp)) {
		if((e=strchr(buf, '#'))) {
			if(*(e-1) != '\\') { *e = 0; }
		}

		if((e=strchr(buf, '\n'))) {
			if(e[-1] == '\r') e[-1] = 0;
			e[0] = 0;
		}

		if((e=strchr(buf, '='))) {
			val = getval(&var, buf);

#define A(x) if(!strcmp(var, #x)) strncpy(cfg->x, val, sizeof(cfg->x))
			A(server);
			else A(nickname);
			else A(username);
			else A(userinfo);
			else A(ircpass);
			else if(!strcmp(var, "port")) cfg->port = atoi(val);
#undef A
		}
	}
	fclose(fp);
	return 0;
}

char *getval(char **var, char *buf)
{
	char *p = buf, *t;
	int c;

	while((c = *p++)) {
		if(c == ' ') {
			for(t = p-1; *t != 0; t++) { t[0] = t[1]; }
		}
	}
	t = strchr(buf, '=');
	*(t++) = 0;
	*var = buf;

	return t;
}

int fake_init(void)
{
	return 0;	/* SUCCESS */
}

void fake_fini(void)
{
}
