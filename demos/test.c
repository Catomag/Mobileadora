#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include <string.h>
#include "../include/mobileadora.h"


void wait(unsigned int milliseconds) {
	struct timespec delay;
	delay.tv_nsec = milliseconds * 1000000;
	delay.tv_sec = 0;
	struct timespec remaining;
	nanosleep(&delay, &remaining);
}


//-----------------------------------------------------------------------

#define MAX_USERS 10

typedef struct {
	char name[21];
} User;

int main() {
	int i = 0;
	User users[MAX_USERS];
	bool b1;
	Frame* frame;
	Frame* frame2;

	ma_init(MAX_USERS, 8000);

	frame = ma_frame_create(FRAME_DYNAMIC, ORIENTATION_VERTICAL, 0, 0);
	ma_frame_input_button_add(frame);
	ma_frame_input_button_add(frame);
	ma_frame_input_button_add(frame);

	frame2 = ma_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL, 0, 0);
	ma_frame_element_h1_add(frame2, "WHAT IS YOUR NAME?");
	ma_frame_element_break_add(frame2);
	ma_frame_element_break_add(frame2);
	ma_frame_element_break_add(frame2);
	ma_frame_element_break_add(frame2);
	ma_frame_input_text_add(frame2, 20);
	ma_frame_input_submit_add(frame2);

	for(i = 0; i < MAX_USERS; i++)
		users[i].name[0] = 0;

	ma_frame_default(frame);
	ma_frame_send(frame, 0);

	while(1) {
		for(i = 0; i < ma_client_max_count(); i++) {
			if(ma_client_active(i)) {

				if(ma_client_input_button_get(i, 0, &b1)) {
					if(b1) {
						if(users[i].name[0] != 0)
							printf("%s, pressed a button!!\n", users[i].name);
						else
							printf("client: %i, pressed a button!!\n", i);
					}
				}
				if(ma_client_input_button_get(i, 1, &b1)) {
					if(b1) {
						if(users[i].name[0] != 0)
							printf("%s, pressed a slightly different button!!\n", users[i].name);
						else
							printf("client: %i, pressed a slightly different button!!\n", i);
					}
				}
				if(ma_client_input_button_get(i, 2, &b1)) {
					if(b1) {
						printf("sending frame2 to client: %i, !!\n", i);
						ma_frame_send(frame2, i);
					}
				}

				if(ma_client_input_text_get(i, 0, users[i].name)) {
					if(users[i].name[0] != '\0') {
						printf("Client %i is called %s\n", i, users[i].name);
						ma_frame_send(frame, i);
					}
				}
			}
		}

		wait(10); // pause for 10ms
	}

	ma_frame_destroy(frame);
	ma_frame_destroy(frame2);
	ma_deinit();
	return 0;
}
