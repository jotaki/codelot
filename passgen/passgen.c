/*
 * a simple password generator
 * don't forget: this must be rewritable/repeatable anywhere/anyhow.
 * unfortunately, I couldn't remember it, after some time, so it's on github now.
 *
 * No warranty.
 * Feel free to reuse this code however you wish.
 * Written by Joseph T. Kinsella
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

/*
 * use first four primes: 2, 3, 5, 7
 * next, find: prime_n based on these primes
 * prime[2] = 3, prime[3] = 5, prime[5] = 11, prime[7] = 17
 * next, find Mp from our list.
 * M₃ = 7, M₅ = 31, M₁₁ = 2047, M₁₇ = 131071
 *
 * Alphabet + Numbers + Symbols = 2*26 + 10 + 32 = 94 + 1 (space)
 * 95 / 5 = 19 M₁₉ = 524287 << should be a good enough period.
 *
 * Find pk ≡ 1 (mod M₁₉)
 *   ι(M₁₉, M₃) = 449389
 *   ι(M₁₉, M₅) = 33825
 *   ι(M₁₉, M₁₁) = 150089
 *   ι(M₁₉, M₁₇) = 174761
 *
 * use Lehmer rng method of X[k+1] = X[k] * a % m
 *   where a = ι(M₁₉, ...), and m = M₁₉
 *
 * slight twist:
 *   let f(x) represent the sequence: 1, 2, 1, 3, 2, 1, 4, 3, 2, 1, ...
 *   a, and m are selected via X[f(prime[x % y]) % 4]
 *   y starts at 1, when x % y = 0, y increases by 1.
 *   when y reaches 16, y resets to 1.
 *   x is chosen by current state:
 *     x = *state/log_e(*state) (~π(*state))
 */
static int primes[16] = {  2,  3,  5,  7, 11, 13, 17, 19,
			  23, 29, 31, 37, 41, 43, 47, 53 };

// A004736 (Too lazy to write my own atm, would probably end up being
// similar anyway.)
static int fseq(int n)
{
	int t = (sqrt(8*n-7) / 2) - 1;
	return (((t*t + 3*n + 4) >> 1) - n);
}

static long int prng(long int *state, int *counter_mod)
{
	static const int _mults[4] = { 449389, 33825, 150089, 174761 };
	static const int mask = 0x7ffff;
	register long int index = (*state / log(*state));

	index %= *counter_mod;
	if(!index)
		*counter_mod = (*counter_mod % 15) + 1;

	index = fseq(primes[index]) % 4;
	return *state = (*state * _mults[index]) & mask;
}

int main(int argc, char *argv[])
{
	int i, counter = 1, using_letters = 0, using_quiet_letters = 0;
	long int state = 0, tmp;
	char *seed, *nl;	/* nl=newline */
	char *letters = NULL;
	char lletters[] = /* legacy letters */
		"`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./~" \
                "!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?";

	unsigned int j, length, randseed = 1218, modulus = 1987;
	size_t letters_length = sizeof(lletters)/sizeof(lletters[0]);
	register int rc = 0;

	if(argc > 1) {
		for(i = 1; i < argc; ++i) {
			if(!strcmp("-s", argv[i])) {
				if(++i >= argc) {
					printf("Need value for seed.\n");
					++rc;
					goto out;
				}
				if(sscanf(argv[i], "%d", &randseed) != 1) {
					printf("Invalid seed value: %s\n", argv[i]);
					++rc;
					goto out;
				}
			}
			else if(!strcmp("-m", argv[i])) {
				if(++i >= argc) {
					printf("Need value for modulus.\n");
					++rc;
					goto out;
				}
				if(sscanf(argv[i], "%d", &modulus) != 1) {
					printf("Invalid modulus value: %s\n", argv[i]);
					++rc;
					goto out;
				}
			}
			else if(!strcmp("-h", argv[i])) {
				printf("Valid options are:\n");
				printf("-s <seed>    -- srand() seed value.\n");
				printf("-m <modulus> -- modulus value.\n");
				printf("-l <letters> -- letters.\n");
				printf("-L           -- ");
				printf("read letters from stdin.\n");
				printf("-q           -- ");
				printf("read letters using getpass().\n");
				printf("             -- ");
				printf("must be used with -L\n");
				printf("-h           -- shows this help.\n");
				printf("\n");
				goto out;
			}
			else if(!strcmp("-l", argv[i])) {
				if(++i >= argc) {
					printf("Need value for letters.\n");
					printf("Use for stingy passphrases.\n");
					++rc;
					goto out;
				}
				letters = argv[i];
				letters_length = strlen(letters);
			}
			else if(!strcmp("-L", argv[i])) {
				letters = calloc(512, sizeof(char));
				if(!letters) {
					printf("Could not allocate 512 bytes.\n");
					++rc;
					goto out;
				}
				using_letters = 1;
			}
			else if(!strcmp("-q", argv[i])) {
				using_quiet_letters = 1;
			}

		}
	}


	seed = getpass("Seed password: ");
	printf("Enter length: ");
	fflush(stdout);

	while(scanf("%d", &length) != 1);
	(void) fgetc(stdin);	// chomp newline.

	if(using_letters) {
		if(!using_quiet_letters) {
			printf("Enter letters: ");
			fflush(stdout);
			if(fgets(letters, 511, stdin) == NULL) {
				printf("Failed to fgets()\n");
				free(letters);
				return 1;
			}

			nl = strchr(letters, '\n');
			if(nl) *nl = '\0';
			
			nl = strchr(letters, '\r');
			if(nl) *nl = '\0';

			if(strlen(letters) == 0) {
				printf("No letters. Aborting.\n");
				free(letters);
				return 1;
			}
		} else {
			free(letters);
			letters = getpass("Enter letters: ");
			// perhaps this variable should be called
			// calloced_letters? *shrugs* it works.
			using_letters = 0;

			nl = strchr(letters, '\n');
			if(nl) *nl = '\0';

			nl = strchr(letters, '\r');
			if(nl) *nl = '\0';

		}
		letters_length = strlen(letters);
	} else if(!letters) {
		letters = lletters;
	}

	srand(randseed);
	for(j = 0; j < strlen(seed); ++j) {
		state += seed[j] + (rand() % modulus);
		tmp = prng(&state, &counter);
		state += tmp;
	}

	for(j = 0; j < length; ++j) {
		printf("%c",
			letters[prng(&state, &counter) % letters_length]);
		fflush(stdout);
	}
	printf("\n");

out:
	if(using_letters)
		free(letters);

	return rc;
}
