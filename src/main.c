// THIS FILE IS ONLY HERE FOR TESTING PURPOSES
#include "../include/library.h"


// Some basic game
int main() {
	library_init(10, 8000);
	Frame* frame = library_frame_create();

	Input text = {
		INPUT_TEXT,
		100
	};

	Input other = {
		INPUT_JOYSTICK,
		2
	};

	library_frame_input_add(frame, text);
	library_frame_input_add(frame, other);
	library_frame_print(frame);
}
