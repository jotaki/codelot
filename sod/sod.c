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
	const struct music_mode_s modes[] = {
		{ "Aelion", 10662 },
		{ "Locrian", 10857 },
		{ "Ionian", 6810 },
		{ "Dorian", 9894 },
		{ "Phrygian", 10665 },
		{ "Lydian", 6762, },
		{ "Mixolydian", 9882 },
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
