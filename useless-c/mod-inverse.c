#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

int64_t modinverse(int64_t b, int64_t m)
{
	int64_t n = 1;

	while((n*b)%m != 1)
		++n;

	return n;
}

int main()
{
	int64_t e, m;

	printf("Enter e=");
	fflush(stdout);
	scanf("%" PRId64, &e);

	printf("Enter m=");
	fflush(stdout);
	scanf("%" PRId64, &m);

	printf("n * %" PRId64 " %% %" PRId64 " = 1; n = %" PRId64 "\n",
			e, m, modinverse(e, m));
	return 0;
}
