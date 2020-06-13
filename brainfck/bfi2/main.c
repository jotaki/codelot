#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>

#include "bfi2.h"

int main(int argc, char *argv[])
{
	char *code = NULL;
	struct machine *machine = NULL;
	bool climode = false;
	char *path;

	if(!machine_create(&machine)) {
		perror("could not create machine");
		return 1;
	}

	if(argc == 1) {
		int rc = cli(machine, false);
		
		machine_destroy(&machine);
		return rc;
	}

	path = argv[1];
	if(argv[1][0] == '-' && argv[1][1] == 'd') {
		climode = true;

		if(argc < 3) {
			printf("Usage: %s [-d <file>]\n", argv[0]);
			machine_destroy(&machine);
			return 1;
		}

		path = argv[2];
	}

	FILE *fp = fopen(path, "r");
	if(!fp) {
		perror("error");
		fprintf(stderr, "failed to open ``%s''\n", path);
		machine_destroy(&machine);
		return 1;
	}

	long size;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if(size <= 0) {
		fprintf(stderr, "empty file ``%s''\n", path);
		fclose(fp);
		machine_destroy(&machine);
		return 0;
	}

	code = calloc(size+1, sizeof(char));
	if(!code) {
		perror("error");
		fprintf(stderr, "cannot allocate buffer (%ld). out of memory?\n", size);
		fclose(fp);
		machine_destroy(&machine);
		return 1;
	}

	int length = 0, ch;
	while((ch = fgetc(fp)) != EOF)
		code[length++] = ch;

	code[length] = '\0';
	fclose(fp);

	if(brainfuck_compile(machine, code) < 0) {
		fprintf(stderr, "failed to compile code.\n");
		fflush(stdout);

		free(code);
		machine_destroy(&machine);
		return 1;
	}

	free(code);
	code = NULL;

	if(climode == false) {
		raw(true, false);
		machine_run(machine);
		raw(false, false);
	} else
		cli(machine, true);

	machine_destroy(&machine);

	return 0;
}
