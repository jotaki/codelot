#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define __USE_BSD
#include <math.h>

#include "aop.h"

/*
 * == Parameters:
 *	+interval+::	Interval from `A'
 *	+octave+::	Octave to use.
 */

float convert_note(int interval, int octave)
{
	float s = 55.00;

	if(octave > 1)
		s *= (1 << (octave - 1));

	return s * powf(2, ((float)interval / 12));
}

/*
 * == Parameters:
 *	+note+::	A string like: "<note>[sharpen|flatten][<octave>]" ->
 *			"A1", "C#1", "Eb1", "A##3", "C"
 */
float note2freq(const char *note)
{
	int interval, octave;
	int decrease = 0, increase = 0;

	switch(toupper(*note)) {
		case 'A': interval = 0x00; break;
		case 'B': interval = 0x02; break;
		case 'C': interval = 0x03; break;
		case 'D': interval = 0x05; break;
		case 'E': interval = 0x07; break;
		case 'F': interval = 0x08; break;
		case 'G': interval = 0x0a; break;
		default: return (float) -1;
	}

	if(!*(++note))	/* we'll assume the 4th octave */
		return convert_note(interval, 4);

	while(*note) {
		if(*note != '#' && *note != 'b') {
			note++;
			break;
		}

		interval += (*note == '#'? 1: -1);
		if(interval == -1) {
			decrease += 1;
			interval = 0x0b;
		} else if(interval == 0x0c) {
			increase += 1;
			interval = 0x00;
		}

		test_printf(("[I] Interval = %d\n", interval));
		note++;
	}
	/* ok, maybe this is ugly. */
	note--;

	octave = atoi(note);
	if(octave == 0)
		octave = 4;

	/* Push to A5 if G##4 was entered *
	 * Drop to G4 if Abb5 was entered *
	 * Do nothing if Ab# was entered */
	octave += (increase + -decrease);

	/* boundries */
	if(octave < 1)
		octave = 1;

	if(octave > 8)
		octave = 8;

	return convert_note(interval, octave);
}

#ifdef TESTCODE

int main(int argc, char *argv[])
{
	int i;
	char *inputs[] = { "A", "Ab", "A##", "C", "C#5", "D2" };
	char *bad_inputs[] = { "1A", "#A1", "Zb", "-", "qwerty" };
	float outputs[] = { 440, 415, 494, 523, 1109, 147 };
	float freq;

	for(i = 1; i < argc; ++i) {
		printf("[I] %s = %f\n", argv[i], note2freq(argv[i]));
	}

	for(i = 0; i < 5; ++i) {
		freq = round(note2freq(inputs[i]));
		printf("[I] \"%s\" == %f (%s)\n", inputs[i], outputs[i],
				(freq == outputs[i]? "PASSED": "FAILED"));
	}

	for(i = 0; i < 5; ++i) {
		freq = note2freq(bad_inputs[i]);
		printf("[I] Processing \"%s\" -- Should Fail ... %s\n",
				bad_inputs[i],
				(freq == -1? "PASSED": "FAILED"));

	}

	return 0;
}

#endif
