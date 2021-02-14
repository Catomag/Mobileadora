// THIS FILE IS ONLY HERE FOR TESTING PURPOSES
#include "../include/library.h"
#include <unistd.h>


// Simple form
int main() {
	l_init(10, 8000);

	Frame* main_frame = l_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL);
	l_frame_input_add(main_frame, l_input_joystick_create());
//	l_send(main_frame, )

	// Contents of first frame
//	l_frame_text_add(main_frame, "First name:");
//	l_frame_input_add(main_frame, l_input_text_create(20));
//	l_frame_text_add(main_frame, "Last name:");
//	l_frame_input_add(main_frame, l_input_text_create(30));
//	l_frame_input_add(main_frame, l_input_submit_create("Submit"));
//
//	// wait 10 seconds for clients to join
//	sleep(10);
//	unsigned int client_count = l_client_count();
//	for(int i = 0; i < client_count; i++) {
//		l_send(main_frame, i);
//	}
//
//	// wait 20 seconds
//	sleep(20);
//	l_frame_fetch(main_frame); // force clients to submit data
//	l_poll(); // update internal data
//
//	char first_client_name[20];
//	l_input_text_get(0, 0, first_client_name);
//
//	char names[20 * client_count];
//	char last_names[30 * client_count];
//	for(int i = 0; i < client_count; i++) {
//		l_input_text_get(i, 0, &names[20 * i]);
//		l_input_text_get(i, 1, &last_names[20 * i]);
//	}
	
	// do stuff with data
	l_frame_print(main_frame);
	l_send(main_frame, 10);

	// exit
	sleep(30);
	l_frame_destroy(main_frame);
	l_free();
}
