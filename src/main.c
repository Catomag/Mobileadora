// THIS FILE IS ONLY HERE FOR TESTING PURPOSES
#include "../include/library.h"


// Some basic game
int main() {
	Frame* frame = library_frame_create();

	Input text = {
		INPUT_TEXT,
		100
	};

	Input other = {
		INPUT_VECTOR,
		4
	};

	library_frame_input_add(frame, text);
	library_frame_input_add(frame, other);
	library_frame_print(frame);
}
