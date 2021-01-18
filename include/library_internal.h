#define _LIBRARY_INTERNAL_
#include "library.h"

struct _Frame {
	FrameType type;
	Orientation orientation;
	unsigned int input_count;
	unsigned int input_size;
	Input* inputs;
	unsigned char* input_data;
};
