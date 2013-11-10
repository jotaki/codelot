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
 *   gcc -W -Wall -s -O2 -o bfi bfi.c -std=c99 -march=native
 *   or just:
 *   gcc -o bfi bfi.c
 *
 * Invocation:
 *   $ ./bfi 99bottles.bf
 *   $ ./bfi <your script>
 *
 * Testing:
 *   $ time ./bfi long.b
 *
 * Notes:
 *   compared to: http://mazonka.com/brainf/
 *   Tested on: Intel(R) Core(TM) i5-2520M CPU @ 2.50GHz
 *
 *   compilation options: gcc -o <f> <f>.c
 *   -------------------------------------
 *   $ time cat long.b | ./bff
 *   real	0m18.820s
 *   user	0m18.725s
 *   sys	0m0.088s
 *
 *   $ time cat long.b | ./bff4
 *   real	0m27.354s
 *   user	0m27.183s
 *   sys	0m0.161s
 *
 *   $ time cat long.b | ./bff4lnr
 *   real	0m7.327s
 *   user	0m7.302s
 *   sys	0m0.024s
 *
 *   $ time ./bfi long.b
 *   real	0m19.400s
 *   user	0m19.399s
 *   sys	0m0.001s
 *
 *   compilation options: gcc -s -O2 -o <f> <f>.c -march=native
 *   ----------------------------------------------------------
 *   $ time cat long.b | ./bff
 *   real	0m13.213s
 *   user	0m13.150s
 *   sys	0m0.060s
 *
 *   $ time cat long.b | ./bff4
 *   real	0m10.003s
 *   user	0m10.003s
 *   sys	0m0.001s
 *
 *   $ time cat long.b | ./bff4lnr
 *   real	0m2.957s
 *   user	0m2.955s
 *   sys	0m0.003s
 *
 *   $ time ./bfi long.b
 *   real	0m10.061s
 *   user	0m10.061s
 *   sys	0m0.000s
 *
 *   compiled with: gcc -o bfi bfi.c -DCLOCK_IT
 *   ------------------------------------------
 *   $ ./bfi long.b 
 *   Read source in 0ms
 *   Compiled source in 0ms
 *   Interpreted brainf*** in 19700ms
 *
 *   Maybe one day I will implement a linear optimization. :-/
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

#ifdef CLOCK_IT
# include <time.h>
#endif

#define MAX(a, b)	((a) > (b)? (a): (b))
#define MIN(a, b)	((a) < (b)? (a): (b))
#define NOT_BF(ch) 	( \
				((ch) != '[') && ((ch) != ']') && \
				((ch) != '+') && ((ch) != '-') && \
				((ch) != '<') && ((ch) != '>') && \
				((ch) != ',') && ((ch) != '.') \
			)

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

char *read_source(const char * const filepath, int *size)
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

	*size = fileinfo.st_size + 1;
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

int *quick_compile(const char * const source, int *size)
{
	int *src, *tmp, *loop, t;
	const char *ptr;
	char byte;

	src = calloc(*size+1, sizeof(int));
	if(src == NULL)
		return NULL;

	tmp = src;
	ptr = source;
	while(*ptr) {
		byte = *tmp = *ptr;
		if(NOT_BF(byte)) {
			++ptr;
			continue;
		}

		t = 0;
		if(byte == ']') {
			loop = tmp;
			do {
				if((*loop & 0xff) == ']')
					++t;
				else if((*loop & 0xff) == '[')
					--t;

				--loop;
			} while(t != 0);
			t = tmp-loop;
		} else if(byte != '[') {
			for(t = 0; *ptr == byte; ++ptr, ++t)
				/* do nothing */ ;
			--ptr;
		}

		*tmp++ |= (t << 8);
		++ptr;
	}
	*size = tmp-src+1;
	return src;
}
			
int interpret_bf(const int * source, char * const memory)
{
	int ptr = 0, i, si = 0;

	while(source[si]) { 
		i = (source[si] >> 8);
		switch((source[si] & 0xff)) {
			case '>': 
				ptr = MIN(MAX_ARRAY_SIZE, ptr + i);
				break;

			case '<':
				ptr = MAX(0, ptr - i);
				break;

			case '+':
				memory[ptr] = MIN(255, memory[ptr] + i);
				break;

			case '-':
				memory[ptr] = MAX(0, memory[ptr] - i);
				break;

			case ']':
				if(memory[ptr])
					si -= i;
				break;

			case ',':
				memory[ptr] = fgetc(stdin);
				break;

			case '.':
				fputc(memory[ptr], stdout);
			  	break;
		}
		++si;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	char *source, *memory;
	int *compiled_src, size;
	int rc = 0;
#ifdef CLOCK_IT
	clock_t start, stop;
#endif

	if(argc < 2) {
		printf("Usage: %s <file.bfp>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

#ifdef CLOCK_IT
	start = clock();
#endif
	source = read_source(argv[1], &size);
	if(source == NULL)
		exit(EXIT_FAILURE);

#ifdef CLOCK_IT
	stop = clock();
	printf("Read source in %ldms\n",
			(long)(stop - start) * 1000 / CLOCKS_PER_SEC);
	start = clock();
#endif

	compiled_src = quick_compile(source, &size);
	if(compiled_src == NULL) {
		free(source);
		exit(EXIT_FAILURE);
	}
#ifdef CLOCK_IT
	stop = clock();
	printf("Compiled source in %ldms\n",
			(long)(stop - start) * 1000 / CLOCKS_PER_SEC);
#endif

	free(source);
	memory = calloc(MAX_ARRAY_SIZE, sizeof(char));
	if(memory == NULL) {
		perror("Out of Memory");
		free(compiled_src);
		exit(EXIT_FAILURE);
	}

#ifdef CLOCK_IT
	start = clock();
#endif
	rc = interpret_bf(compiled_src, memory);
#ifdef CLOCK_IT
	stop = clock();
	printf("Interpreted brainf*** in %ldms\n",
			(long)(stop - start) * 1000 / CLOCKS_PER_SEC);
#endif

	free(memory);
	free(compiled_src);
	return rc;
}
