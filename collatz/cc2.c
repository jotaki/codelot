/*
 * just checking a thing
 */
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#define f(n) ((int64_t)ceil((((n) & 1) + 0.5) * (n)) << ((n) & 1))
#define f2(n) (((int64_t)((n) << ((n) & 1)) - ((n) >> 1)) << ((n & 1)))

inline int64_t f3(int64_t n)
{
	if((n & 1) == 0)
		n >>= 1;
	else
		n = 3 * n + 1;

	return n;
}


int main()
{
	int k;

	for(k = 1; k <= 10000000; ++k)
		if(f3(k) != f(k) || f(k) != f2(k)) {
			printf("%d failed.\n", k);
			break;
		}

	return k < 10000000;
}
