#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <ao/ao.h>

#define __USE_BSD
#include <math.h>

#include "aop.h"

/*
 * == Parameters:
 *	+note+::	Note to be passed to note2freq()
 *	+device+::	Device to output audio to.
 *	+format+::	Output format.
 *	+buffer+::	Buffer to place audio data into.
 *	+buf_size+::	Size of buffer.
 */	
int play_note(const char *note, ao_device *device, ao_sample_format *format,
		char *buffer, int buf_size)
{
	float freq;
	int i, sample;

	freq = note2freq(note);
	printf("Playing freq: %f\n", freq);

	for(i = 0; i < format->rate; ++i) {
		sample = (int)(24576.0 * sin(2*M_PI*freq *
					((float)i/format->rate)));

		memcpy(&buffer[4*i], &sample, sizeof(sample));
	}

	return ao_play(device, buffer, buf_size);
}
