#ifndef MKSHELL_H
# define MKSHELL_H

# define AS_PATH	"/usr/bin/as"
# define LD_PATH	"/usr/bin/ld"

int assemble_object(const char *appname, const char *input,
		const char *output);
int link_object(const char *appname, const char *input, const char *output);

int mkshell(const char *input);

#endif
