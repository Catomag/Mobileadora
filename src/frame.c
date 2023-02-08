#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/mobileadora_internal.h"





Frame* default_frame = NULL;





// Frame binary file (very efficient)
//
// |--------------------------HEADER----------------------------------------------------------------------------------|---------INPUT-0-0-------|---INPUT-N---|--repeat for elem->
// [[type_header]-[frame_type]-[frame_orientation]-[frame_resizeable]-[frame_scrollable]-[input_count]-[element_count] [input_type]-[input-size] [    ...    ]]
//	4 bits,	   	  1 bit,	   1 bit,			   1 bit,	 	 	  1 bit			 	 1 byte,	   1 byte			 1 byte,	  4 bytes.
//
// |------------------------------------------------------------------------------------------------------------------|-------------------------|-------------|

void ma_frame_send(Frame* frame, unsigned int client_index) {
	assert(frame != NULL || default_frame != NULL);
	assert(client_index < clients_size);

	if(frame == NULL)
		frame = default_frame;

	// assign frame to client
	if(clients[client_index].frame == NULL)
		clients[client_index].input_data = realloc(clients[client_index].input_data, frame->input_size);
	else if(clients[client_index].frame->input_size < frame->input_size)
		clients[client_index].input_data = realloc(clients[client_index].input_data, frame->input_size);

	bzero(clients[client_index].input_data, frame->input_size);
	clients[client_index].frame = frame;

	// construct file
	unsigned char input_count   = frame->input_count;
	unsigned char element_count = frame->element_count;

	// 3 bytes for header, 5 bytes per input, 5 bytes per element and element size because of internal data stored
	unsigned int frame_size = 3 + frame->raw_data_size;
	unsigned char* frame_data = malloc(frame_size);

	// set up header
	frame_data[0] = 0;
	frame_data[0] |= MESSAGE_FRAME		<< 4;
	frame_data[0] |= frame->type 		<< 3;
	frame_data[0] |= frame->orientation << 2;
	frame_data[0] |= frame->resizeable  << 1;
	frame_data[0] |= frame->scrollable;

	input_count   = frame->input_count;
	element_count = frame->element_count;

	// copy info onto frame data
	memcpy(&frame_data[1], &input_count, sizeof(char));
	memcpy(&frame_data[2], &element_count, sizeof(char));
	memcpy(&frame_data[3], frame->raw_data, frame->raw_data_size);

	// send file
	ma_send(clients[client_index].socket_fd, frame_data, frame_size);
	free(frame_data);
}




void ma_frame_default(Frame* frame) {
	default_frame = frame;
}




Frame* ma_frame_create(FrameType type, Orientation orientation, bool scrollable, bool resizeable) {
	Frame* frame = (Frame*) malloc(sizeof(Frame));
	frame->type = type;
	frame->orientation = orientation;
	frame->scrollable = scrollable;
	frame->resizeable = resizeable;
	frame->inputs = NULL;
	frame->input_size = 0;
	frame->input_count = 0;
	frame->elements = NULL;
	frame->element_data = NULL;
	frame->element_count = 0;
	frame->element_size = 0;
	frame->element_allocated = 0;
	frame->raw_data = NULL;
	frame->raw_data_size = 0;
	frame->raw_data_allocated = 0;

	return frame;
}




Frame* ma_frame_copy(Frame* frame) {
	assert(frame != NULL || default_frame != NULL);
	if(frame == NULL)
		frame = default_frame;

	Frame* frame_copy = (Frame*) malloc(sizeof(Frame));

	memcpy(frame_copy, frame, sizeof(Frame));

	// create a copy of every element of data

	frame_copy->inputs = malloc(frame->input_count * sizeof(Input));
	memcpy(frame_copy->inputs, frame->inputs, frame->input_count * sizeof(Input));

	frame_copy->elements = malloc(frame->element_count * sizeof(Element));
	memcpy(frame_copy->elements, frame->elements, frame->element_count * sizeof(Element));

	frame_copy->element_data = malloc(frame->element_allocated);
	memcpy(frame_copy->element_data, frame->element_data, frame->element_size);

	frame_copy->raw_data = malloc(frame->raw_data_allocated);
	memcpy(frame_copy->raw_data, frame->raw_data, frame->raw_data_size);

	//ma_frame_print(frame);
	//ma_frame_print(frame_copy);

	return frame_copy;
}



// When frame is destroyed, all clients are kicked back into the default frame
void ma_frame_destroy(Frame* frame) {
	if(default_frame == frame)
		default_frame = NULL;

	for(int i = 0; i < clients_size; i++) {
		if(clients[i].frame == frame) {
			clients[i].frame = default_frame;

			// if a frame is destoyed, client is redirected to the default_frame
			if(default_frame != NULL)
				ma_frame_send(default_frame, i);
		}
	}

	if(frame->inputs != NULL)
		free(frame->inputs);

	if(frame->elements != NULL)
		free(frame->elements);

	if(frame->raw_data != NULL)
		free(frame->raw_data);

	free(frame);
}




void ma_frame_input_add(Frame* frame, Input input) {
	frame->inputs = realloc(frame->inputs, sizeof(Input) * frame->input_count + sizeof(Input)); // this might be expensive

	frame->inputs[frame->input_count].type = input.type;
	frame->inputs[frame->input_count].size = input.size;

	frame->input_count++;
	frame->input_size += input.size;

	// add to the raw data
	if(frame->raw_data_allocated <= frame->raw_data_size + 5) {
		frame->raw_data_allocated = (frame->raw_data_size + sizeof(Input)) * 2;
		frame->raw_data = realloc(frame->raw_data, frame->raw_data_allocated);
	}

	memcpy(&frame->raw_data[frame->raw_data_size + 0], &input.type, sizeof(char));
	memcpy(&frame->raw_data[frame->raw_data_size + 1], &input.size, sizeof(int));
	frame->raw_data_size += 5;
}




void ma_frame_element_add(Frame* frame, Element element, void* data) {
	frame->elements = (Element*) realloc(frame->elements, sizeof(Element) * frame->element_count + sizeof(Element));

	frame->elements[frame->element_count].type = element.type;
	frame->elements[frame->element_count].size = element.size;

	// allocate data
	if(frame->element_allocated <= frame->element_size + element.size) {
		frame->element_allocated = (frame->element_size + element.size) * 2;
		frame->element_data = realloc(frame->element_data, frame->element_allocated);
	}

	if(element.size > 0 && data != NULL)
		memcpy(frame->element_data + frame->element_size, data, element.size);

	frame->element_count++;
	frame->element_size += element.size;

	// add to the raw data
	if(frame->raw_data_allocated <= frame->raw_data_size + element.size + sizeof(Element)) {
		frame->raw_data_allocated = (frame->raw_data_size + element.size + sizeof(Element)) * 2;
		frame->raw_data = realloc(frame->raw_data, frame->raw_data_allocated);
	}

	Element e = element;
	e.type |= 0x80; // make the most significant bit be 1, so that client can distinguish them

	memcpy(&frame->raw_data[frame->raw_data_size + 0], &e.type, sizeof(char));
	memcpy(&frame->raw_data[frame->raw_data_size + 1], &e.size, sizeof(int));
	memcpy(&frame->raw_data[frame->raw_data_size + 5], data, element.size);
	frame->raw_data_size += 5 + element.size;
}




void ma_frame_element_set(Frame* frame, Element element, unsigned char index, void* data) {
	void* current_byte = frame->element_data;
	Element current_element = frame->elements[0];
	unsigned char current_index = 0;

	// increase memory if necessary
	if(frame->element_allocated < frame->element_size + element.size) {
		frame->element_allocated = (frame->element_size + element.size) * 2;
		frame->element_data = realloc(frame->element_data, frame->element_allocated);
	}

	for(unsigned short i = 0; i < frame->element_count; i++) {
		current_element = frame->elements[i];

		// if type is matched
		if(current_element.type == element.type) {
			// if index is the same
			if(current_index == index) {
				unsigned int original_size = current_element.size;

				// move existing data forward / backward
				unsigned int bytes_left = frame->element_size - (current_byte + original_size - frame->element_data);
				memmove(current_byte + original_size, current_byte + (element.size - original_size), bytes_left);
				// now copy the new data
				memcpy(current_byte, data, element.size);

				// make sure that the element_size doesn't include the data we removed
				current_element.size = element.size;
				frame->element_size -= original_size;
				frame->element_size += element.size;

				break;
			}
			current_index++;
		}

		current_byte += current_element.size;
	}

	// the same change must be done to the raw_data
	unsigned char* raw_data = frame->raw_data;
	unsigned long byte = 0;
	current_index = 0;

	while(byte < frame->raw_data_size) {
		// if the data is an element
		if(raw_data[byte] & 0x80) {
			unsigned int current_element_size = *((unsigned int*) &raw_data[byte + 1]);
			// if the element is the type we want
			if((raw_data[byte] & 0x7f) == element.type) {
				// if the element is the index we want
				if(current_index == index) {
					unsigned int original_size = current_element_size;

					// now copy the new data
					memcpy(frame->raw_data + 5 + byte, data, element.size);
					break;
				}

				current_index++;
			}
			// only increment size if its an element as inputs don't store data in the array
			byte += 5 + current_element_size;
		}
		else
			byte += 5;
	}
}




void ma_frame_print(Frame* frame) {
	printf("orientation: %s\n", (frame->orientation == ORIENTATION_VERTICAL) ? "vertical" : "horizontal");
	printf("type: %s\n",		(frame->type == FRAME_STATIC) ? "static" : "dynamic");
	printf("scrollable: %s\n", 	(frame->scrollable) ? "true" : "false");
	printf("resizeable: %s\n", 	(frame->resizeable) ? "true" : "false");

	for(unsigned int i = 0; i < frame->input_count; i++) {
		printf("	%i Input: %i, length: %u\n", i, frame->inputs[i].type, frame->inputs[i].size);
	}

	unsigned int byte = 0;
	for(unsigned int i = 0; i < frame->element_count; i++) {
		printf("	%i Element: %i, length: %u, data:\n", i, frame->elements[i].type, frame->elements[i].size);

		for(unsigned int j = 0; j < frame->elements[i].size; j++)
			printf("		0x%X\n", ((unsigned char*) frame->element_data)[byte + j]);

		byte += frame->elements[i].size;
	}

	printf("input size: %u\n", frame->input_size);
	printf("input count: %u\n", frame->input_count);
	printf("element size: %u\n", frame->element_size);
	printf("element count: %u\n", frame->element_count);
}

