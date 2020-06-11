#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int factor(int n)
{
	int f;

	for(f = sqrt(n); n % f != 0; --f)
		/* do nothing */ ;

	return f;
}

int adjust(int n, int *a, int *b)
{
	*a = factor(n);
	*b = n / *a;

	if(*a == 1) {	// prime
		*a = sqrt(n);
		*b = n / *a;
	}

	return n - *a * *b;
}

int main(int argc, char *argv[])
{
	char line[1024];
	struct {
		int outside;
		int inside;
	} block;

	while(fgets(line, sizeof(line), stdin) != NULL) {
		char *p;
		int pad = 0;	// padding (used for primes)

		//if((p = strchr(line, '\n'))) *p = '\0';
		//if((p = strchr(line, '\r'))) *p = '\0';

		for(p = line; *p; ++p) {
			pad = adjust(*p, &block.outside, &block.inside);

			int i;

			printf(">");
			for(i = 0; i < block.outside; ++i) printf("+");

			printf("[<");
			for(i = 0; i < block.inside; ++i) printf("+");

			printf(">-]<");
			for(i = 0; i < pad; ++i) printf("+");

			printf(".>");
		}
		printf("\n");
	}
	return 0;
}
