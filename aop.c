#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <ao/ao.h>

#define __USE_BSD
#include <math.h>

/*
 * == Parameters:
 *	+interval+::	Interval from `A'
 *	+octave+::	Octave to use.
 */

float convert_note(int interval, int octave)
{
	float s = 55.00;

	if(octave > 1)
		s = 55.00 * (2 * octave);

	return s * powf(2, ((float)interval / 12));
}

/*
 * == Parameters:
 *	+note+::	A string like: "[octaveN][noteL][sharpen]" ->
 *			"1A", "4G#", "3C"
 */
float note2freq(const char *note)
{
	unsigned int octave, pitch, i, n = 0;
	unsigned int intervals[] = {
		0x0041,	/* A */ 0x2341, /* A# */ 0x0042, /* B */
		0x0043,	/* C */ 0x2343, /* C# */ 0x0044, /* D */
		0x2344, /* D# */ 0x0045, /* E */ 0x0046, /* F */
		0x2346,	/* F# */ 0x0047, /* G */ 0x2347, /* G# */
	};

	memcpy((void *) &n, (const void *) note, sizeof(n));

	octave = (n & 0xff) - 0x30;
	pitch  = (n & 0x00ffff00) >> 8;

	for(i = 0; i < 12; ++i) {
		if(intervals[i] == pitch)
			return convert_note(i, octave);
	}

	/* fall back to A1 */
	return convert_note(0, 0);
}


int play_note(const char *note, ao_device *device, ao_sample_format *format,
		char *buffer, int buf_size)
{
	float freq;
	int i, sample;

	freq = note2freq(note);
	for(i = 0; i < format->rate; ++i) {
		sample = (int)(24576.0 *
					sin(2*M_PI*freq * ((float)i/format->rate)));

		memcpy(&buffer[4*i], (const void *) &sample, sizeof(sample));
	}

	return ao_play(device, buffer, buf_size);
}


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
