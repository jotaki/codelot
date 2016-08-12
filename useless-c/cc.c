/*
 * Collatz conjecture:
 *
 * f(2n)   -> n/2
 * f(2n+1) -> (3n+1)/2
 *
 * (3n+1)/2 = (n + n + n + 1)/2
 *         = ((n + n) + (n + 1))/2
 *         = (2n/2) + (n+1)/2
 *         = n + (n+1)/2
 *         = 2n - (n/2)
 *         = n + (n/2)
 *         = 1n + 0.5n
 *         = ceil(1.5n)
 * (n/2)   = n * 0.5
 *
 * f(2n+1) -> ceil(n * 1.5)
 * f( 2n ) -> ceil(n * 0.5)
 *
 * f(n) -> ceil(((n % 2) + 0.5) * n)
 * -or for 3n+1-
 * f(n) -> ((n % 2) + 1) * ceil(((n % 2) + 0.5) * n)
 *
 * f(4n-1) => f(4x+2)
 * f(4n+1) => f(4x)
 * f(4n+2) => f(2x)
 * f(4n)   => f(2x)
 */
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <sys/time.h>

#define f(n) ((int64_t)ceil((((n) & 1) + 0.5) * (n)) << ((n) & 1))

/* brute force method */
int64_t G(int64_t n)
{
	int64_t k;

	for(k = 0; n != 1; ++k)
		n = f(n);

	return k;
}

/* loop around 4n+-1 */
int64_t g(int64_t n)
{
	int64_t a,b,q,s,k = 0;

	if(n > 1 && !(n & 1)) {
		k += __builtin_ctz(n);
		n >>= k;
	}

	if(n <= 1)
		return k;

	while(n != 1) {
		q = (n >> 1) & 1;
		a = (n + 1) >> 2;
		b = ((n - a) << 2) | (q << 1);
		s = __builtin_ctz(b);
		k = 1 + k + s;
		n = b >> s;
	}

	return k;
}

/* wrapper test code; returns the time it took to finish in ms */
uint64_t testit(int64_t (*p)(), int64_t max, int64_t *p_chains, int64_t *p_n)
{
	int64_t i, chains = 0, n = 0, tmp;
	struct timeval begin = {0}, end = {0};

	gettimeofday(&begin, NULL);
	for(i=1; i <= max; ++i) {
		tmp = p(i);
		if(chains < tmp) {
			chains = tmp;
			n = i;
		}
	}
	gettimeofday(&end, NULL);

	if(p_chains) *p_chains = chains;
	if(p_n) *p_n = n;

	return (uint64_t) (end.tv_usec - begin.tv_usec);
}


inline void x_testit(const char *fn, int64_t (*p)(), int64_t max)
{
	int64_t chains, n;
	uint64_t r;

	printf("Testing from %s(1) to %s(%"PRId64"):\n", fn, fn, max);
	r = testit(p, max, &chains, &n);

	printf("Took %"PRIu64"ms to complete %"PRId64" loops.\n", r, max);
	printf("Found %"PRId64" with the longest chain count of %"PRId64"\n",
			n, chains);
}

int main()
{
	int64_t in, i_fn = 0;
	int64_t (*p)(int64_t) = G;
	char *fn[] = { "G", "g", NULL };

	while(scanf("%"PRId64, &in) == 1) {
		if(in == 0) {
			scanf("%"PRId64, &in);
			i_fn = (in & 1);
			p = (i_fn)? g: G;
			continue;
		} else if(in < 0) {
			x_testit(fn[i_fn], p, -in);
			continue;
		}

		printf("f(%"PRId64")=%"PRId64"; ", in, f(in));
		printf("%s(%"PRId64")=%"PRId64"\n", fn[i_fn], in, p(in));
	}
	return 0;
}
