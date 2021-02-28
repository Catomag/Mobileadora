#include "../include/library_internal.h"
#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <assert.h>

// input definitions
const Input JOYSTICK = {
	INPUT_JOYSTICK,
	sizeof(short) * 2
};

const Input BUTTON = {
	INPUT_BUTTON,
	sizeof(unsigned char)
};

// TODO: revise this, using clients_data right now so that new data is only accessed when l_poll occurs
bool l_input_get(unsigned int client_index, InputType type, unsigned char input_index, unsigned char* data) {
//	unsigned long index = 0;
//	unsigned int input_size_sum = 0;
//	unsigned char inputs_of_type_count = 0;
//
//	if(client_index >= 0 && client_index < clients_size) {
//		for(unsigned int i = 0; client_index > 0 && i < client_index; i++) {
//			index += clients[i].frame->input_size;
//		}
//
//		for(unsigned char i = 0; i < clients[client_index].frame->input_count; i++) {
//			if(clients[client_index].frame->inputs[i].type == type) {
//				if(inputs_of_type_count == input_index) {
//					for(unsigned int j = 0; j < clients[client_index].frame->inputs[i].size; j++)
//						data[j] = (unsigned char) clients_data[index + j];
//
//					return 1;
//				}
//				inputs_of_type_count++;
//			}
//
//			input_size_sum += clients[client_index].frame->inputs[i].size;
//		}
//	}

	unsigned char current_index = 0;
	unsigned long current_byte = 0;
	for(unsigned char i = 0; i < clients[client_index].frame->input_count; i++) {
		if(clients[client_index].frame->inputs[i].type == type) {
			if(input_index == current_index) {
				memcpy(data, &clients[client_index].input_data[current_byte], clients[client_index].frame->inputs[i].size);
				return 1;
			}
			current_index++;
		}
		current_byte += clients[client_index].frame->inputs[i].size;
	}
	return 0;
}


Input l_input_joystick_create() {
	return JOYSTICK;
}

bool l_input_joystick_get(unsigned int client_index, unsigned char input_index, float* x_value, float* y_value) {
	if(clients[client_index].input_data == NULL)
		return 0;

	unsigned char data[JOYSTICK.size];
	if(l_input_get(client_index, INPUT_JOYSTICK, input_index, data)) {
		short x = be16toh(data[0]);
		short y = be16toh(data[2]);
		*x_value = ((float) x / 0xFFFF) * 2.f;
		*y_value = ((float) y / 0xFFFF) * 2.f;

		if(*x_value > .99)
			*x_value = 1;
		else if(*x_value < -.99)
			*x_value = -1;

		if(*y_value > .99)
			*y_value = 1;
		else if(*y_value < -.99)
			*y_value = -1;

		return 1;
	}
	else
		return 0;
}

Input l_input_button_create() {
	return BUTTON;
}

bool l_input_button_get(unsigned int client_index, unsigned char input_index, unsigned char* value) {
	if(clients[client_index].input_data == NULL)
		return 0;

	unsigned char data;
	if(l_input_get(client_index, INPUT_BUTTON, input_index, &data)) {
		*value = data;
		return 1;
	}
	else
		return 0;
}
