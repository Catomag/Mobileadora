#define _LIBRARY_INTERNAL_
#include "library.h"

struct _Frame{
	unsigned int input_count;
	unsigned int input_size;
	Input* inputs;
	unsigned char* input_data;
};
