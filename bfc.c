#include <stdio.h>
#include <string.h>

void start_code()
{
	printf(".section .text\n");
	printf(".global _start\n");
	printf("_start:\n");
	printf("movq $.mem, %%rsi\n");
}

void end_code()
{
	printf("movq $60, %%rax\n");
	printf("xorq %%rdi, %%rdi\n");
	printf("syscall\n");
	printf(".section .data\n");
	printf(".mem:\n");
	printf(".zero 30000\n");
}

void op(int cnt, int isp)
{
	if(cnt == 1 || cnt == -1) {
		printf("%s %s%%rsi%s\n",
				0 > cnt? "decq": "incq",
				isp? "(": "",
				isp? ")": "");
		return;
	}

	printf("%s $%d, %s%%rsi%s\n",
			0 > cnt? "subq": "addq",
			0 > cnt? -cnt: cnt,
			isp? "(": "",
			isp? ")": "");
}

void printch()
{
	printf("movq $1, %%rax\n");
	printf("movq $1, %%rdi\n");
	printf("movq $1, %%rdx\n");
	printf("syscall\n");
}

void readch()
{
	static int r = 0;

	printf("jmp .readchr%d_in\n", r);
	printf(".readchr%d_bad:\n", r);
	printf("movq $0, (%%rsi)\n");
	printf("jmp .readchr%d_out\n", r);
	printf(".readchr%d_in:\n", r);
	printf("xorq %%rax, %%rax\n");
	printf("xorq %%rdi, %%rdi\n");
	printf("movq $1, %%rdx\n");
	printf("syscall\n");
	printf("cmpq $0, %%rax\n");
	printf("jz .readchr%d_bad\n", r);
	printf(".readchr%d_out:\n", r);

	++r;
}

void tagloop(int i)
{
	printf(".loop%d:\n", i);
}

void jmploop(int i)
{
	printf("cmpb $0, (%%rsi)\n");
	printf("jnz .loop%d\n", i);
}

int main()
{
	int c, l, i = 0;
	char src[30000] = {0};

	while((c=fgetc(stdin))!=EOF) {
		if(i > sizeof(src)-1) {
			break;
		}

		else if(strchr("[<,+-.>]", c) == NULL) {
			continue;
		}

		src[i++] = c;
	}

	start_code();
	for(i = 0; src[i]; ++i) {
		c = 1;
		if(src[i] == '[' || src[i] == ']')
			goto _next;

		for(; src[i] == src[i+1]; ++i, ++c)
			/* do nothing */ ;

_next:
		switch(src[i]) {
			case '<': op(-c, 0); break;
			case '-': op(-c, 1); break;
			case '>': op(c, 0); break;
			case '+': op(c, 1); break;
			case '.': printch(); break;
			case ',': readch(); break;
			case '[': tagloop(i); break;
			case ']': 
				  l = 0;
				  c = i;
				  do {
					  if(src[c] == ']') ++l;
					  else if(src[c] == '[') --l;

					  --c;
				  } while(l != 0);
				  jmploop(c+1);
				  break;
		}
	}
	end_code();

	return 0;
}
