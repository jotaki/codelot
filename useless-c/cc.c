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
 *
 * g(1) -> 0
 * g(2n) -> __builtin_ctz(n) + g(n >> __builtin_ctz(n))
 * g(2n+1) -> ((n % 4) + 1)/2 + g(3 * n + 1)
 *
 * f(7) = 7*3+1 = 22
 * f(22)= 22 / 2 = 11
 * f(11) = 11*3+1 = 34
 * f(34) = 34/2 = 17
 * f(17) = 17*3+1 = 52
 * f(52) = 52 / 2 = 26
 * f(26) = 26 / 2 = 13
 * f(13) = 13*3+1 = 40
 * f(40) = 40 / 2 = 20
 * f(20) = 20 / 2 = 10
 * f(10) = 10 / 2 = 5
 * f(5) = 5*3+1 = 16
 * f(16) = 16 / 2 = 8
 * f(8) = 8 / 2 = 4
 * f(4) = 4 / 2 = 2
 * f(2) = 2 / 2 = 1
 * f(1) = 1*3+1 = 4
 * f(4) = 4 / 2 = 2
 * f(2) = 2 / 2 = 1
 * f(1) = ...
 *
 * f(4n-1) => f(4x+2)
 * f(4n+1) => f(4x)
 * f(4n+2) => f(2x)
 * f(2n)   => f(n/2)
 *
 * n/2 = 2m+1 when n = 4x+2
 * n/2 = 2m when n = 4x
 */
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

int64_t f(int64_t n)
{
#if 0
	if((n & 1) == 1)
		n = 3 * n + 1;
	else
		n >>= 1;

	return n;
#endif
	return (int64_t) ceill(((n & 1) + 0.5) * n) << (n & 1);
}

int64_t ff(int64_t n)
{
	if(n > 0 && !(n & (n - 1)))
		return __builtin_ctz(n);

	return 1 + ff(f(n));
}

int64_t fff(int64_t n)
{
	int64_t r;

	if(n <= 0)
		return 0;

	for(r = 0; n & (n - 1); ++r)
		n = (int64_t) ceil(((n & 1) + 0.5) * n) << (n & 1);

	return r + __builtin_ctz(n);
}

int64_t g(int64_t n)
{
	int64_t i;

	if(n == 1)
		return 0;

	else if((n & 1) == 0) {
		i = __builtin_ctz(n);
		return i + g(n>>i);
	
	}

	i = 3 * n + 2;
	if((i % 4) == 0)
		return 2 + g(i-1);

	return 1 + g(i - 1);
}

int main()
{
	int64_t in, k, n, m;

	while(scanf("%" PRId64, &in) == 1) {
		printf("f(%" PRId64 ") = %" PRId64 "; ", in, f(in));

		for(k = 0, n = in; n != 1; k++)
			n = f(n);

		printf("k = %" PRId64 "; ", k);
		printf("k = %" PRId64 "? ", ff(in));
		printf("k = %" PRId64 "? ", fff(in));
		printf("k = %" PRId64 "?\n", g(in));
	}

	k = m = in = 0;
	for(n = 1; n <= 1000000; ++n) {
		k = g(n);
		if(k > m) {
			m = k;
			in = n;
		}
	}
	printf("%" PRId64 " has %" PRId64 " chains.\n", in, m);
	return 0;
}
