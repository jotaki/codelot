#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void usage(appname, exit_code)
	const char * const appname;
	int exit_code;
{
	fprintf(stderr, "Usage: %s <server>:<port> <script>\n", appname);
	exit(exit_code);
}

int connection_info(info, server, port)
	char * info;
	char ** server;
	char ** port;
{
	*server = info;
	*port   = strchr(info, ':');

	if(*port == NULL)
		return 1;

	*((*port)++) = '\0';
	return (0 >= atoi(*port));
}

int main(argc, argv, envp)
	int argc;
	char *argv[];
	char *envp[];
{
	struct stat info;
	struct addrinfo *result, *rp, hints = {0};
	char *server, *port;
	int err, sd = -1, flag;

	if(3 > argc)
		usage(argv[0], EXIT_FAILURE);

	if(connection_info(argv[1], &server, &port))
		usage(argv[0], EXIT_FAILURE);

	if(0 > stat(argv[2], &info)) {
		perror(argv[2]);
		exit(EXIT_FAILURE);
	}

	if((~info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
		fprintf(stderr, "%s is not executable\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	err = getaddrinfo(server, port, &hints, &result);
	if(err != 0) {
		if(err == EAI_SYSTEM) {
			perror("getaddrinfo");
			exit(EXIT_FAILURE);
		}

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(0 > sd)
			continue;

		flag = 1;
		setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, flag);

		if(connect(sd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;

		close(sd);
	}

	if(rp == NULL) {
		fprintf(stderr, "failed to connect.\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);
	close(STDERR_FILENO);
	if(0 > dup2(sd, STDIN_FILENO) || 0 > dup2(sd, STDOUT_FILENO)) {
		perror("dup2");
		close(sd);
		exit(EXIT_FAILURE);
	}

	if(0 > execve(argv[2], &argv[2], envp)) {
		close(sd);
		exit(EXIT_FAILURE);
	}

	/* should not make it here */
	close(sd);
	exit(EXIT_SUCCESS);
	return 0;
}
