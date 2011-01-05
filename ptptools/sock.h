#ifndef SOCK_H
# define SOCK_H

# include <sys/time.h>
# include <sys/select.h>

int skgetc(int *);
int skgets(char *, int, int *);
int sel_skgets(char *, int, int, fd_set *, struct timeval *);

int skputs(char *, int *);
int skprintf(int *, char *, ...);

int *skconnect(char *, short);

void skclose(int *);

#endif
