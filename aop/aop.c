/*
 * linkage:
 *	http://www.techlib.com/reference/musical_note_frequencies.htm
 *
 * Compile with gcc -W -Wall -s -O2 -pedantic -std=c99 -o aop aop.c -lm -lao
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ao/ao.h>

#include "aop.h"

/*
 * Main entry point...
 * Example Usage:
 *	./aop C E G F A C B D F E G B A C E D F A G B D C E G
 */
int main(int argc, char *argv[])
{
	ao_device *device;
	ao_sample_format format;
	int default_driver, i, buf_size;
	char *buffer;

	ao_initialize();
	default_driver = ao_default_driver_id();

	memset(&format, 0, sizeof(format));
	format.bits = 16;
	format.channels = 2;
	format.rate = 44100;
	format.byte_format = AO_FMT_LITTLE;

	device = ao_open_live(default_driver, &format, NULL);
	if(device == NULL) {
		fprintf(stderr, "Error opening device.\n");
		return 1;
	}

	buf_size = format.bits / 8 * format.channels * format.rate;
	buffer = calloc(buf_size, sizeof(char));
	if(buffer == NULL) {
		ao_close(device);
		fprintf(stderr, "Out of memory.\n");
		return 1;
	}

	for(i = 1; i < argc; ++i) {
		play_note(argv[i], device, &format, buffer, buf_size);
	}

	free(buffer);
	ao_close(device);
	ao_shutdown();

	return 0;
}
