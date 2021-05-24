#include "../include/library_internal.h"
#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <assert.h>

#include <Gaia/Hephaestus.h>

// input definitions
const Input BUTTON = {
	INPUT_BUTTON,
	sizeof(unsigned char)
};

const Input SUBMIT = {
	INPUT_BUTTON_SEND,
	0
};

const Input TOGGLE = {
	INPUT_BUTTON,
	sizeof(unsigned char)
};

const Input JOYSTICK = {
	INPUT_JOYSTICK,
	sizeof(float) * 2
};


// TODO: revise this, using clients_data right now so that new data is only accessed when ma_poll occurs
bool ma_input_get(unsigned int client_index, InputType type, unsigned char input_index, void* data) {
	if(clients[client_index].input_data == NULL)
		return 0;

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

	//h_debug_println();
	for(unsigned int i = 0; i < clients[client_index].frame->input_count; i++) {
		if(clients[client_index].frame->inputs[i].type == type) {
			if(input_index == current_index) {
				unsigned int input_size = clients[client_index].frame->inputs[i].size;
				if(data != NULL)
					memcpy(data, &clients[client_index].input_data[current_byte], input_size);

				return 1;
			}
			current_index++;
		}
		current_byte += clients[client_index].frame->inputs[i].size;
	}
	return 0;
}






// Frame input declarations


void ma_frame_input_generic_add(Frame* frame, unsigned int size) {
	Input input;
	input.type = INPUT_GENERIC;
	input.size = size;
	ma_frame_input_add(frame, input);
}

void ma_frame_input_text_add(Frame* frame, unsigned int max_chars) {
	Input input;
	input.type = INPUT_TEXT;
	input.size = max_chars + 1;
	ma_frame_input_add(frame, input);
}

void ma_frame_input_button_add(Frame* frame) {
	ma_frame_input_add(frame, BUTTON);
}

void ma_frame_input_submit_add(Frame* frame) {
	ma_frame_input_add(frame, SUBMIT);
}

void ma_frame_input_toggle_add(Frame* frame) {
	ma_frame_input_add(frame, TOGGLE);
}

void ma_frame_input_joystick_add(Frame* frame) {
	ma_frame_input_add(frame, JOYSTICK);
}


// Client input declarations
bool ma_client_input_generic_get(unsigned int client_index, unsigned char input_index, void* value) {
	if(ma_input_get(client_index, INPUT_GENERIC, input_index, NULL)) {
		ma_input_get(client_index, INPUT_GENERIC, input_index, value);
		return 1;
	}
	else
		return 0;
}

bool ma_client_input_text_get(unsigned int client_index, unsigned char input_index, char* value) {
	if(ma_input_get(client_index, INPUT_TEXT, input_index, NULL)) {
		ma_input_get(client_index, INPUT_TEXT, input_index, (unsigned char*) value);
		return 1;
	}
	else
		return 0;
}

bool ma_client_input_button_get(unsigned int client_index, unsigned char input_index, bool* value) {
	bool data;
	if(ma_input_get(client_index, INPUT_BUTTON, input_index, &data)) {
		*value = data;
		return 1;
	}
	else
		return 0;
}

bool ma_client_input_toggle_get(unsigned int client_index, unsigned char input_index, bool* value) {
	bool data;
	if(ma_input_get(client_index, INPUT_TOGGLE, input_index, &data)) {
		*value = data;
		return 1;
	}
	else
		return 0;
}

bool ma_client_input_joystick_get(unsigned int client_index, unsigned char input_index, float* x_value, float* y_value) {
	// should probably find a better approach
	unsigned char data[JOYSTICK.size];
	if(ma_input_get(client_index, INPUT_JOYSTICK, input_index, data)) {
		*x_value = *((float*) &data[0]);
		*y_value = *((float*) &data[4]);

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
