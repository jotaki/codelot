#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LIST_SIZE	25 * 1024 * 1024

/* min by compare */
inline int mbc(int a, int b) { return a > b? b: a; }

/* min by subtract */
inline int mbs(int a, int b) { return b + ((a - b) & ((a - b) >> 31)); }

/* compare with compare */
int cwc(const void * p1, const void * p2)
{
	if(*(int*)p1 == *(int*)p2)
		return 0;
	else if(mbc(*(int*)p1, *(int*)p2) == *(int*)p1)
		return -1;

	return 1;
}

/* compare with subtraction */
int cws(const void * p1, const void * p2)
{
	if(*(int*)p1 == *(int*)p2)
		return 0;
	else if(mbs(*(int*)p1, *(int*)p2) == *(int*)p1)
		return -1;

	return 1;
}

#if 0
void print(int * list, int size) {
	int i;

	printf("[ ");
	for(i = 0; i < size; ++i) {
		printf("%d ", list[i]);
		if((i+1)%12 == 0)
			printf("\n");
	}
	printf(" ]\n");
}
#else
# define print(a,b)
#endif

/* declarations */
int * genlist(int size);
void bm_sort(int * list, int size, int (*cmp)());

int main()
{
	int * list1, * list2;

	list1 = genlist(LIST_SIZE);
	if(list1 == NULL) {
		perror("genlist()");
		return -1;
	}

	list2 = malloc(LIST_SIZE * sizeof(int));
	if(list2 == NULL) {
		perror("malloc()");
		free(list1);
		return -1;
	}
	memcpy(list2, list1, LIST_SIZE * sizeof(int));

	printf("Verifying copy ... ");
	if(memcmp(list1, list2, LIST_SIZE * sizeof(int)) != 0) {
		printf("bad\n");
		goto _done;
	}
	printf("good\n");

	printf("Sorting using comparison\n");
	bm_sort(list1, LIST_SIZE, cwc);
	print(list1, LIST_SIZE);

	printf("Sorting using 'subtraction'\n");
	bm_sort(list2, LIST_SIZE, cws);
	print(list2, LIST_SIZE);

	printf("Verifying sort ... ");
	if(memcmp(list1, list2, LIST_SIZE * sizeof(int)) != 0) {
		printf("bad\n");
		goto _done;
	}
	printf("good\n");

_done:
	free(list1);
	free(list2);
	return 0;
}

/* not the greatest algorithm, but it works. */
void bm_sort(int * list, int size, int (*cmp)())
{
	clock_t start, stop;

	start = clock();
	qsort(list, size, sizeof(int), cmp);
	stop = clock();

	printf("Sorted %d in %ld ms\n", size,
			(long)(stop - start) * 1000 / CLOCKS_PER_SEC);
}

int * genlist(int size)
{
	int * ret = malloc(size * sizeof(int));
	int i;

	if(ret) {
		srand(time(NULL));
		for(i = 0; i < size; ++i) {
			ret[i] = rand() & 0xffff;
		}
	}
	return ret;
}

