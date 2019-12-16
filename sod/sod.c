#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct music_mode_s {
	char *mm_mode;
	int mm_scale;
};

int main(int argc, char *argv[])
{
	time_t seed;
	const char *chromatic[] = {
		"A", "A#", "B", "C", "C#", "D",
		"D#", "E", "F", "F#", "G", "G#",
	};
	const struct music_mode_s modes[] = {	/* backwards, missing last */
		{ "Aelion", 10662 },	/* 2212212 */
		{ "Locrian", 10857 },	/* 2221221 */
		{ "Ionian", 6810 },	/* 1222122 */
		{ "Dorian", 9894 },	/* 2122212 */
		{ "Phrygian", 10665 },	/* 2212221 */
		{ "Lydian", 6762, },	/* 1221222 */
		{ "Mixolydian", 9882 },	/* 2122122 */
	};
	int scale, mode, tmp, root;

	time(&seed);
	srand(seed);

	mode = rand() % (sizeof(modes)/sizeof(modes[0]));
	root = rand() % (sizeof(chromatic)/sizeof(chromatic[0]));

	scale = modes[mode].mm_scale;
	tmp = root;

	printf("%s %s: ", chromatic[root], modes[mode].mm_mode);
	while(scale != 0) {
		printf("%s ", chromatic[tmp]);
		tmp += (scale & 3);
		tmp %= (sizeof(chromatic)/sizeof(chromatic[0]));
		scale >>= 2;
	}
	printf("%s\n", chromatic[root]);

	return 0;
}
