/*
 * Filename: bfi.c
 * Date: 2013-09-24
 * Description: Very simple brainf**k interpreter.
 * Reason: I was bored one night.
 *
 * Copyright? nah! feel free to do whatever you want with this code, just know
 * if it takes over your computer and suddenly your carpet comes to life to
 * attack you, it's not my fault.
 *
 * Compilation:
 *   gcc -W -Wall -s -O2 -o bfi bfi.c -ansi -D_BSD_SOURCE -march=native
 *   or just:
 *   gcc -o bfi bfi.c
 *
 * Invocation:
 *   ./bfi 99bottles.bf
 *   ./bfi <your script>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

enum {
	MAX_ARRAY_SIZE = 30000,
};

void perrorf(const char * const fmt, ...)
{
        va_list ap;
        int err = errno;

        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);

        fprintf(stderr, ": %s\n", strerror(err));
        fflush(stderr);
}

char *read_source(const char * const filepath)
{
	char *data = NULL;
	struct stat fileinfo;
	int fd = -1, n, i = 0;

	fd = open(filepath, O_RDONLY);
	if(0 > fd) {
		perrorf("open('%s', O_RDONLY)", filepath);
		return NULL;
	}

	if(0 > fstat(fd, &fileinfo)) {
		perrorf("fstat(%d)", fd);
		goto finish;
	}

	data = malloc(fileinfo.st_size + 1);
	if(data == NULL) {
		perrorf("malloc(%lld)", fileinfo.st_size);
		goto finish;
	}
	memset(data, 0, fileinfo.st_size + 1);

	do {
		n = read(fd, &data[i], fileinfo.st_size - i);
		if(0 > n) {
			perrorf("read(%d, %p, %lld)",
					fd, &data[i],
					fileinfo.st_size - i);
			free(data);
			data = NULL;
			goto finish;
		}
		i += n;
	} while(i != fileinfo.st_size);

finish:
	if(0 > fd)
		close(fd);

	return data;
}

int interpret_bf(const char * const source, char * const memory)
{
	const char *src = source;
	int ptr = 0, encounter = 0;
	register int r;

	while(*src) {
		switch(*src) {
			case '>':
				++ptr;
				break;

			case '<':
				--ptr;
				break;

			case '+':
				++memory[ptr];
				break;

			case '-':
				--memory[ptr];
				break;

			case ',':
				/* handle error? */
				r = read(STDIN_FILENO, &memory[ptr], 1);
				if(0>r) {
					fsync(STDIN_FILENO);
				}
				break;

			case '.':
				/* handle error? */
				r = write(STDOUT_FILENO, &memory[ptr], 1);
				if(0 > r) {
					fsync(STDOUT_FILENO);
				}
				break;

			case ']':
				encounter = 0;
				if(memory[ptr]) {
					do {
						if(*src == ']') {
							++encounter;
						} else if(*src == '[') {
							--encounter;
						}
						--src;
					} while(encounter != 0);
				}
				break;
		}
		++src;
	}

	return 0;
}

int main(int argc, char *argv[])
{
        char *source, *memory;
        int rc = 0;

        if(argc < 2) {
                printf("Usage: %s <file.bfp>\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        source = read_source(argv[1]);
        if(source == NULL)
                exit(EXIT_FAILURE);

        memory = calloc(MAX_ARRAY_SIZE, sizeof(char));
        if(memory == NULL) {
                perror("Out of Memory");
                free(source);
                exit(EXIT_FAILURE);
        }

        rc = interpret_bf(source, memory);

        free(memory);
        free(source);
        return rc;
}
