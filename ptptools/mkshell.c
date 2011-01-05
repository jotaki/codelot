#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mkshell.h"

#ifdef AS_APP

int main(int argc, char *argv[])
{
	char *appname;
	char obj[16] = "/tmp/objXXXXXX";
	char bin[16] = "/tmp/binXXXXXX";

	if((appname = strrchr(argv[0], '/')) != NULL)
		++appname;
	else
		appname = argv[0];

	if(argc < 2) {
		fprintf(stderr, "usage: %s <file.s>\n", appname);
		return 1;
	}

	mkstemp(obj);

	if(assemble_object(appname, argv[1], obj)) {
		unlink(obj);
		return 1;
	}

	mkstemp(bin);

	if(link_object(appname, obj, bin)) {
		unlink(obj);
		unlink(bin);
		return 1;
	}

	if(mkshell(bin)) {
		fprintf(stderr, "%s: failed.\n", appname);
		unlink(obj);
		unlink(bin);
		return 1;
	}

	unlink(obj);
	unlink(bin);

	return 0;
}

#endif

int assemble_object(const char *appname, const char *input,
		const char *output)
{
	pid_t parent;
	int status;

	parent = fork();
	if(parent > 0) {
		while(waitpid(parent, &status, 0) > 0)
			usleep(2500);
	} else if(parent == 0) {
		if(execl(AS_PATH, "as", "--32", "-o", output, input, NULL))
			perror(appname);
	} else {
		fprintf(stderr, "%s: failed to fork().\n", appname);
		return 1;
	}

	return 0;
}

int link_object(const char *appname, const char *input, const char *output)
{
	pid_t parent;
	int status;

	parent = fork();
	if(parent > 0) {
		while(waitpid(parent, &status, 0) > 0)
			usleep(2500);
	} else if(parent == 0) {
		execl(LD_PATH, LD_PATH, "--oformat=binary", "-o", output, input, NULL);
	} else {
		fprintf(stderr, "%s: could not fork().\n", appname);
		return 1;
	}

	return 0;
}

int mkshell(const char *input)
{
	FILE *fp;
	int ch, loc = 0;

	fp = fopen(input, "r");
	if(fp == NULL)
		return 1;

	printf("#include <stdio.h>\n");
	printf("#include <stdlib.h>\n");
	printf("#include <string.h>\n\n");

	printf("static char shellcode[] = \n\t\"");
	while((ch = fgetc(fp)) != EOF) {
		if((loc++ % 14) == 0 && loc != 1) printf("\"\n\t\"");
		printf("\\x%02x", ch);
	}
	printf("\";\n\nint main(int argc, char *argv[])\n{\n\t/* code here */\n}\n");

	fclose(fp);
	return 0;
}
