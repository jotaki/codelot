#ifndef AOP_H_
# define AOP_H_

# include <ao/ao.h>

float convert_note(int interval, int octave);
float note2freq(const char *note);

int play_note(const char *note, ao_device *device, ao_sample_format *format,
		char *buffer, int buf_size);

# ifdef TESTCODE
#  define test_printf(x)	printf x
# else
#  define test_printf(x)
# endif

#endif	/* AOP_H_ */
