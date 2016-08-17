#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>

void goodbye()
{
	printf("\nGoodbye.\n");
	fflush(stdout);
	exit(0);
}

int64_t gcd(int64_t a, int64_t b)
{
	int64_t t;

	if(b == 0)
		return a;

	while(b) {
		t = b;
		b = a % b;
		a = t;
	}
	return a;
}

int64_t modinverse(int64_t b, int64_t m)
{
	int64_t t, x = 0, y = 1, mm = m;

	if(m == 1)
		return 0;

	while(b > 1) {
		// gcc -O compiles this down to one idivq operation.
		t = x;
		x = y - ((b / m) * x);
		y = t;

		t = m;
		m = b % m;
		b = t;

	}

	if(y < 0)
		y += mm;

	return y;
}

int main()
{
	int64_t a, b, c;

	signal(SIGINT, goodbye);

	while(1) {
		printf("* %%: ");
		fflush(stdout);

		if(scanf("%" PRId64 " %" PRId64, &a, &b) == EOF) {
			printf("\nGoodbye.\n");
			fflush(stdout);
			break;
		}

		c = gcd(a,b);
		if(c > 1) {
			a /= c;
			b /= c;

			printf("Input values are not coprime. Using %" \
				PRId64 " and %" PRId64 " instead.\n", a, b);
		}

		c = modinverse(a, b);
		printf("%" PRId64 " * %" PRId64 " %% %" \
			PRId64 " = %" PRId64 "\n", c, a, b, c*a%b);
	}
	return 0;
}
