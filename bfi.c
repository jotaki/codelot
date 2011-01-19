/*
 * simple Brainfsck interpretter
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_SOURCE	32767
#define MAX_MEMORY	32767

#define TOKEN_ERR	", Invalid token"
#define UNBALANCED_ERR	", Unbalanced brackets"

#define report_syntax_error(line, column, byte, msg) \
	fprintf(stderr, "Syntax error on line #%d,%d : `%c'%s\n", \
			line, column, byte, msg);

static char memory[MAX_MEMORY];

int filesize(FILE *fp)
{
	int size = 0;
	
	/* check return value? */
	fseek(fp, 0, SEEK_END);
	size = (int) ftell(fp);
	fseek(fp, 0, SEEK_SET);

	return size;
}

char *syntax_check(FILE *fp, int allocate)
{
	char *source;
	int ch, open = 0, i = 0;
	int line = 1, col = 0;

	source = calloc(allocate, sizeof(char));
	if(source == NULL) {
		fprintf(stderr, "Out of memory.\n");
		return NULL;
	}

	ch = fgetc(fp);
	while(ch != EOF) {
		switch(ch) {
			case '#':
				while((ch = fgetc(fp)) != '\n' && ch != EOF);
				line++; col = 0;
			break;

			case '>':
			case '<':
			case '+':
			case '-':
			case ',':
			case '.':
			case '[':
			case ']':
				source[i++] = ch;
				open += (ch == '[' ? 1 :
					 ch == ']' ? -1 : 0);
				col++;
			break;

			case '\n':
				line++;
				col = 0;
			break;

			case ' ':
			case '\t':
				col++;
			break;

			default:
				report_syntax_error(line, col, ch, TOKEN_ERR);
				free(source);
				return NULL;

		}

		if(open < 0) {
			report_syntax_error(line, col, ch, UNBALANCED_ERR);
			free(source);
			return NULL;
		}

		ch = fgetc(fp);
	}

	if(open != 0) {
		fprintf(stderr, "unexpected EOF on line %d, expected `]'\n",
				line);
		free(source);
		return NULL;
	}

	/* one could potentially call realloc here, might be worth it
	 * if a brainfsck file had a lot of comments. */
	return source;
}

void interpret(char *source)
{
	char *mem = memory;

	while(*source) {
		switch(*source) {
			case '+': (*mem)++; break;
			case '-': (*mem)--; break;
			case '>': mem++; break;
			case '<': mem--; break;
			case ',': *mem = fgetc(stdin); fflush(stdin); break;
			case '.': fputc(*mem, stdout); fflush(stdout); break;
			case ']': if(*mem) while(*source-- != '['); break;
		}
		source++;
	}
}
	
int main(int argc, char *argv[])
{
	FILE *fp;
	char *buffer;
	int size, r = 0;

	fp = fopen(argv[1], "r");
	if(fp == NULL) {
		fprintf(stderr, "Failed to open %s: %s\n", argv[1],
				strerror(errno));
		return errno;
	}

	/* BUG: includes comments */
	size = filesize(fp);
	if(size > MAX_SOURCE) {
		fprintf(stderr, "Too much source!\n");
		fclose(fp);
		return EFBIG;	/* File too large */
	}

	buffer = syntax_check(fp, size);
	if(buffer) {
		memset(memory, 0, MAX_MEMORY);
		interpret(buffer);
		free(buffer);
	} else
		r = 1;

	fclose(fp);
	return r;
}
