#include <stdio.h>
#include <ctype.h>

#define swap(b)	((((b) & 0xf0) >> 4) | (((b) & 0xf) << 4))

unsigned int hash_op (const char *op)
{
	unsigned int h = 0;

	while (*op)
	{
		h += tolower (*op);
		h += (h << 10);
		h ^= (h >> 6);

		++op;
	}
	h += (h << 3);
	h ^= (h >> 11);
	h += (h << 15);

	return h;
}

int main (int argc, char *argv[])
{
	if (argc > 1)
	{
		printf ("0x%08x\n", hash_op (argv[1]));
	}

	return ( argc == 1 );
}
