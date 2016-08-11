/* Collatz conjecture
 * f(2n)   -> n/2
 * f(2n+1) -> (3n+1)/2
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

#define f(n) ((int64_t)ceil((((n) & 1) + 0.5) * (n)) << ((n) & 1))

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
		q = (n % 4) >> 1;
		a = (n + 1) >> 2;
		b = ((n - a) << 2) + 2*q;
		s = __builtin_ctz(b);
		k = 1 + k + s;
		n = b >> s;
	}

	return k;
}

int main()
{
	int64_t in, k, n, m;

	while(scanf("%"PRId64, &in) == 1) {
		printf("f(%"PRId64") = %"PRId64"; ", in, f(in));

		for(k = 0, n = in; n != 1; k++)
			n = f(n);

		printf("k = %"PRId64"; g(%"PRId64") = %"PRId64"\n",k,in,g(in));
	}
	return 0;
}
