#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

int64_t gcd(int64_t a, int64_t b)
{
	if(b == 0)
		return a;

	return gcd(b, a % b);
}

int64_t modinverse(int64_t b, int64_t m)
{
	int64_t n;

	for(n = 1; (b*n)%m != 1; ++n)
		/* do nothing */ ;

	return n;
}

void encrypt(const char * src, int64_t e, int64_t n)
{
	const char * s = src;

	while(*src) {
		printf("%" PRId64 " ", (int64_t)(powl(*src, e)) % n);
		++src;
	}

	if(n <= 255) {
		src = s;
		printf("\"");
		while(*src) {
			printf("\\x%02x", (char)((int64_t)(powl(*src, e)) % n));
			++src;
		}
		printf("\"\n");
	}
}

/*
 * need to use big int (libmpz?)
 */
void decrypt(const char * src, int64_t d, int64_t n)
{
	int64_t c;

	while(src && sscanf(src, "%" PRId64, &c)) {
		printf("%c", (char)((int64_t)powl(c, d) % n));
		src = strchr(src, ' ');
	}
	printf("\n");
}

#define PROMPT(q,r)\
	printf((q)); fflush(stdout); \
	scanf("%" PRId64, r); fflush(stdin);


int main()
{
	int64_t p, q, n, m, e, d;

	PROMPT("p=", &p);
	PROMPT("q=", &q);
	PROMPT("e=", &e);

	n = p * q;
	m = (p - 1) * (q - 1);
	if(e <= 1) {
		for(e = 3; gcd(e, m) != 1; e += 2)
			/* do nothing */ ;

		printf("e=%" PRId64 "\n", e);
	}
	d = modinverse(e, m);
	printf("d=%" PRId64 "\n", d);
	printf("n=%" PRId64 "\n", n);
	printf("m=%" PRId64 "\n", m);

	do {
		int64_t o = 4;
		
		printf("%24d. Encrypt\n", 1);
		printf("%24d. Decrypt\n", 2);
		printf("%24d. Change values\n", 3);
		printf("%24d. Exit\n", 4);

		PROMPT("Enter option: ", &o);

		if(o == 1) {
			char buffer[1024] = {0};
			printf("Enter message: ");
			fflush(stdout);

			/* eh? */
			fgets(buffer, sizeof(buffer)-1, stdin);
			fgets(buffer, sizeof(buffer)-1, stdin);
			encrypt(buffer, e, n);
		}
		else if(o == 2) {
			char buffer[1024] = {0};
			printf("Enter cipher values: ");
			fflush(stdout);

			/* eh? */
			fgets(buffer, sizeof(buffer)-1, stdin);
			fgets(buffer, sizeof(buffer)-1, stdin);
			decrypt(buffer, d, n);
		}
		else if(o == 3) {
			return main();	// ugly
		}
		else if(o == 4) {
			exit(0);
		}
		else {
			printf("Invalid option: %" PRId64, o);
		}
	} while(1);

	return 0;
}
