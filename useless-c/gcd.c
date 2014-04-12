#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int64_t gcd(int64_t a, int64_t b)
{
	if(b == 0)
		return a;

	return gcd(b, a % b);
}

int main(int argc, char *argv[])
{
	if(argc == 1) {
		printf("Usage:\n\t%s a,b [(a,b)...]\n", argv[0]);
		return 1;
	}

	int i;
	int64_t a, b;

	for(i = 1; i < argc; ++i) {
		sscanf(argv[i], "%" PRId64 ",%" PRId64, &a, &b);
		printf("gcd(%" PRId64 ", %" PRId64 ") = %" PRId64 "\n",
				a, b, gcd(a, b));
	}
	return 0;
}
