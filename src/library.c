#include <stdlib.h>
#include <stdio.h>

#include "../include/library_internal.h"

void l_init(unsigned int max_clients, unsigned short port) {
	// launch server
}


Frame* l_frame_create(FrameType type, Orientation orientation) {
	Frame frame;
	frame.type = type;
	frame.orientation = orientation;
	frame.inputs = malloc(sizeof(Input));
	frame.input_data = malloc(sizeof(unsigned char));
	return malloc(sizeof(Frame));
}

void l_frame_destroy(Frame* frame) {
	free(frame);
}

// TODO: verify this works
void l_frame_input_add(Frame* frame, Input input) {
	frame->inputs = realloc(frame->inputs, sizeof(Input) * (frame->input_count + 1));
	frame->inputs[frame->input_count].type = input.type;
	frame->inputs[frame->input_count].size = input.size;

	unsigned int total_input_size = 0;
	for(unsigned int i = 0; i < frame->input_count; i++) {
		total_input_size += frame->inputs[i].size;
	}

	// generate new array of new length
	frame->input_data = realloc(frame->input_data, sizeof(unsigned char) * total_input_size);

	frame->input_count++;
	frame->input_size += input.size;
}

void l_frame_print(Frame* frame) {
	for(unsigned int i = 0; i < frame->input_count; i++) {
		printf("%i Input: %i, length: %u\n", i, frame->inputs[i].type, frame->inputs[i].size);
	}

	printf("input size: %u\n", frame->input_size);
	printf("input count: %u\n", frame->input_count);
}
