#include "../include/library_internal.h"
#include <stdio.h>

#include <netdb.h>
#include <assert.h>

// input definitions
const Input JOYSTICK = {
	INPUT_JOYSTICK,
	sizeof(short) * 2
};

// TODO: revise this, using clients_data right now so that new data is only accessed when l_poll occurs
bool l_input_get(unsigned int client_index, InputType type, unsigned char input_index, unsigned char* data) {
	unsigned long index = 0;
	unsigned int input_size_sum = 0;
	unsigned char inputs_of_type_count = 0;

	if(client_index > 0 && client_index < clients_size) {
		for(unsigned int i = 0; i < client_index; i++) {
			index += clients[i].frame->input_size;
		}

		for(unsigned char i = 0; i < clients[client_index].frame->input_count; i++) {
			if(clients[client_index].frame->inputs[i].type == type) {
				if(inputs_of_type_count == input_index) {
					for(unsigned int j = 0; j < clients[client_index].frame->inputs[i].size; j++)
						data[j] = (unsigned char) clients_data[index];

					return 1;
				}
				inputs_of_type_count++;
			}

			input_size_sum += clients[client_index].frame->inputs[i].size;
		}
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
//	bool res = l_input_get(client_index, INPUT_JOYSTICK, input_index, data);
	// TODO: remove this HORRIBLE HACK
	for(int i = 0; i < JOYSTICK.size; i++)
		data[i] = clients[client_index].input_data[i];

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

//	return res;
	return 1;
}
