/*
 * file: iwatch.c
 * author: Joseph Kinsella
 * date: 2010-04-26
 * description:
 * 	watch a directory for changes via inotify.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <fcntl.h>

static char *app(char *arg0);
static void usage(char *appname);
static int check_files(char *appname, char *dirent, char *script);
static int exec_iwatch(int fd, char *script);
static int *appstate();
static int fire(struct inotify_event *ev, char *script);
static void handle_signal(int signo);
static char *describe(uint32_t mask);

/*
 * really just an entry point, the good stuff is in exec_iwatch
 */
int main(int argc, char *argv[])
{
	char *appname = NULL;	/* app name */
	char *script = NULL;	/* script file */
	char *dirent = NULL;	/* directory to watch */
	int fd = 0;		/* file descriptor to inotify table */
	int wd = 0;		/* watch descriptor */
	int result = 0;		/* return code */

 	signal(SIGINT, handle_signal);

	appname = app(argv[0]);
	if(argc != 3) {
		usage(appname);
		return (0);
	}

	dirent = argv[1];
	script = argv[2];

	if(check_files(appname, dirent, script)) {
		return (1);
	}

	fd = inotify_init1(IN_CLOEXEC);
	if(fd < 0) {	/* error */
		fprintf(stderr, "uh oh! an error occured. :(\n");
		fprintf(stderr, "inotify_init1(2): %s\n", strerror(errno));
		return (errno);
	}

	wd = inotify_add_watch(fd, dirent, IN_ALL_EVENTS);
	if(wd < 0) {
		perror("inotify_add_watch(2)");
		close(fd);
		return (1);
	}

	result = exec_iwatch(fd, script);

	if(inotify_rm_watch(fd, wd) < 0) {
		perror("inotify_rm_watch(2)");
		result = errno;
	}

	close(fd);
	return (result);
}

/*
 * just strip something like /usr/bin/iwatch to iwatch
 */
static char *app(char *arg0)
{
	char *slash = NULL;

	slash = strrchr(arg0, '/');
	if(slash) {
		return (slash + 1);
	}

	return (arg0);
}

/*
 * print usage information
 */
static void usage(char *appname)
{
	fprintf(stderr, "iwatch: Watch a directory via inotify\n");
	fprintf(stderr, "written by Joseph Kinsella\n\n");
	fprintf(stderr, "Usage: %s <directory> <script>\n", appname);
}

/*
 * check the directory / script entered.
 */
int check_files(char *appname, char *dirent, char *script)
{
	struct stat info;

	if(stat(dirent, &info) < 0) {
		perror("dir: stat(2)");
		return (1);
	}

	if(S_ISDIR(info.st_mode) == 0) {
		fprintf(stderr, "%s: %s: not a directory\n", appname, dirent);
		return (1);
	}

	if(stat(script, &info) < 0) {
		perror("script: stat(2)");
		return (1);
	}

	return (0);
}

/*
 * real execution
 */
static int exec_iwatch(int fd, char *script)
{
	int *as = NULL;
	int result = 0;
	char buffer[BUFSIZ];
	struct inotify_event *event = (struct inotify_event *) buffer;

	as = appstate();
	while(*as == 0) {
		result = read(fd, (void *) buffer, sizeof(buffer));
		if(result < 0) {
			perror("read(2)");
			return (errno);
		}
		fire(event, script);
	}

	return (0);
}

/*
 * simple singleton
 */
static int *appstate()
{
	static int app_state = 0;
	return (&app_state);
}

/*
 * fires script off
 */
static int fire(struct inotify_event *ev, char *script)
{
	pid_t p = 0;
	int status = 0;
	char buffer[BUFSIZ];	/* I can only hope this is big enough. */
	char mask[32];

	p = fork();
	if(p > 0) {	/* parent process */
		do {
			waitpid(p, &status, 0);
		} while(!WIFEXITED(status));
	} else if(p == 0) {
		/* eek, ..but I can't use malloc() since no one would be
		 * able to clean it up. */
		strncpy(buffer, ev->name, ev->len);
		buffer[ev->len] = 0;

		snprintf(mask, sizeof(mask) - 1, "%s", describe(ev->mask));
		execl(script, script, mask, buffer, NULL);
	}

	return (0);
}

/*
 * handles ^C
 */
static void handle_signal(int signo)
{
	fprintf(stderr, "SIGINT caught...\n");
	*(appstate()) = signo;
}

/*
 * return a string description of +mask+
 */
static char *describe(uint32_t mask)
{
	static char buf[32];
	snprintf(buf, 31, "NULL (0x%08x)", mask);

	switch(mask) {
		case IN_ACCESS: return ("IN_ACCESS"); break;
		case IN_ATTRIB: return ("IN_ATTRIB"); break;
		case IN_CLOSE_WRITE: return ("IN_CLOSE_WRITE"); break;
		case IN_CLOSE_NOWRITE: return ("IN_CLOSE_NOWRITE"); break;
		case IN_CREATE: return ("IN_CREATE"); break;
		case IN_DELETE: return ("IN_DELETE"); break;
		case IN_DELETE_SELF: return ("IN_DELETE_SELF"); break;
		case IN_MODIFY: return ("IN_MODIFY"); break;
		case IN_MOVE_SELF: return ("IN_MOVE_SELF"); break;
		case IN_MOVED_FROM: return ("IN_MOVED_FROM"); break;
		case IN_MOVED_TO: return ("IN_MOVED_TO"); break;
		case IN_OPEN: return ("IN_OPEN"); break;
		default: return (buf); break;
	}
	return ("NULL");
}

