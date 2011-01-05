#ifndef H_ZEBOT_
# define H_ZEBOT_

# include "irc.h"
# include "irc_cmd.h"

struct config {
	char filename[256];

	char server[256];
	short port;

	char nickname[32];
	char username[32];
	char userinfo[32];

	char ircpass[64];
	char nspass[64];
};

struct module_ptr {
	char mod_name[256];
	double mod_version;
	char author[256];

	int (*init)();

	void (*on_numeric)();
	void (*on_privmsg)();
	void (*on_notice)();
	void (*on_join)();
	void (*on_part)();
	void (*on_quit)();
	void (*on_kick)();
	void (*on_mode)();

	void (*fini)();

	void *handle;
	char local[2048];
	struct irc irc;

	short mod_index;
	char mod_pf[512];
	struct module_ptr *next;
};

# ifndef __MODULE__

#  define VERSION "ZeBot v0.3"

struct module_ptr *load_module(struct module_ptr *head, const char *path);
void unload_module(struct module_ptr *head, short index, char *path);
void cleanup_module(struct module_ptr **head);
void run_modules(struct module_ptr *head, socket_t *sck, const char *buf,
		struct config *cfg);

void mod_cmd(struct module_ptr *head, socket_t *sck,
		const char *rpl, const char *cmd);

int load_config(struct config *cfg);
char *getval(char **var, char *buf);

int fake_init(void);
void fake_fini(void);

# else	/* __MODULE__ */

char *read_config(FILE *fp, const char *var, void *val);
#  define load_config(file, cfg, var) read_config(file, #var, (cfg)->var)

# endif /* !__MODULE__ */

#endif	/* H_ZEBOT_ */
