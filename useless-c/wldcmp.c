#include <stdio.h>
#include <time.h>

int wldcmp(char *wld, char *str)
{
	int r = 1, k=0;
	char *p;

	for(p=wld;*p;p++) k+=(*p=='*')?0:1;
	if(strlen(str)<k) return 0;

	for(; *str || *wld; str++, wld++) {
		if(*str == *wld)
			continue;
		else if(*wld == '?') {
			k = 0;
			while(*(str-k) == '?') k++;
			if(*(str-k) == *str)
				continue;
			else
				return 0;
		} else if(*wld == '*') {
			if(!*(wld+1)) return 1;

			while(*++wld && r) {
				for(p = str; *p; p++)
					if(*p == *wld) {
						r = wldcmp(wld,p);
						if(r) return 1;
					}
			}
		} else return 0;
	}
	return 1;
}

#define STR_CHECK(w,s)	\
	"check %4i :%s\n", ++i, wldcmp(w,s) ? "passed" : "failed"
#define println()		(fputc('\n', stdout))

void run_cmp_tests(void)
{
	int i = 0;

	printf("should pass:\n");
	printf(STR_CHECK("", ""));
	printf(STR_CHECK("*", ""));
	printf(STR_CHECK("*", "hello world"));
	printf(STR_CHECK("hello world",	"hello world"));
	printf(STR_CHECK("* world", "hello world"));
	printf(STR_CHECK("hello *", "hello world"));
	printf(STR_CHECK("hello*world",	"hello world"));
	printf(STR_CHECK("*!*@*", "zer0python!joseph@comcast.net"));
	println();

	printf("should fail:\n");
	printf(STR_CHECK("hello world", ""));
	printf(STR_CHECK("hello world", "yay"));
	printf(STR_CHECK("hello world", "hello"));
	printf(STR_CHECK("hello world", "world"));
	printf(STR_CHECK("hello world", "hello worlde"));
	printf(STR_CHECK("hello world", "hello worl"));
	println();
}

void run_clock_test(void)
{
	clock_t st, et;
	unsigned long j = 0;

	st = clock();
	et = st + 4 * CLOCKS_PER_SEC;

	while(clock() < et) {
		wldcmp("abcd*", "abcdefgieowru29ur0932uoiejwfoijewfhuweiufhiuwhfiu" \
				"hewhfruwiefru2093r092u3frjoiuewfouhweufhwuehiubiuewhfviuh" \
				"wfuewfiew82ur0u2309rioewjfoiwjefiuhbwreiufbwrefhweqf823ur" \
				"0982u3riuowefoiuwhehfqwiuehfiuqwhefwvg");
		j++;
	}
	printf("%ld loops in 4 secs\n", j);
}

int main(void)
{
	run_cmp_tests();
	run_clock_test();

	return 0;
}
