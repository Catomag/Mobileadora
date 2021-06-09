#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

#include <sys/time.h>
#include <pthread.h>

#include <poll.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>

#include <openssl/sha.h>
#include <string.h>

#include "../include/library_internal.h"

#define _H_DEBUG_MEMORY_
#include <Gaia/Hephaestus.h>

static int server_fd;

Client* clients;
pthread_t client_thread;
unsigned int clients_count;
unsigned int clients_size;
void* clients_data = NULL;
unsigned long clients_data_size = 0;

unsigned char server_running = 0;

Frame* default_frame = NULL;

char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
// output needs to be 28 bytes, data needs to be 20 bytes
void hash_to_base64(unsigned char* data, char* output) {
	unsigned char current_bit = 0;
	unsigned char group_count = 1;
	unsigned char index = 0;

	// there's probably a better way to implement this that is way more clever but its 11
	while(current_bit < 156) {
		unsigned char current_byte = current_bit / 8;
		unsigned char bit_value = (data[current_byte] >> (7 - (current_bit % 8))) & 1;

		index <<= 1;
		index |= bit_value;

		// reached end of group
		if(group_count == 6) {
			output[current_bit / 6] = base64_alphabet[index];
			group_count = 0;
			index = 0;
		}

		group_count++;
		current_bit++;
	}

	// do the special 16 case

	// 0000 1111
	index = (data[19] & 15) << 2;
	output[26] = base64_alphabet[index];
	output[27] = '=';
}

// TODO: maybe add support for multiple dataframes
void* ma_client_handler(void* data) {
	struct timespec delay;
	delay.tv_nsec = 5 * 1000000; // TODO: remember, 20ms of artificial delay
	delay.tv_sec = 0;
	struct timespec remaining;

	unsigned int payload_size = 200;
	unsigned char* payload = malloc(sizeof(unsigned char) * payload_size);

	while(server_running) {

		struct pollfd pollfds[clients_size];
		for(uint i = 0; i < clients_size; i++) {
			pollfds[i].fd = clients[i].socket_fd;
			pollfds[i].events = POLLIN | POLLOUT;
		}
		poll(pollfds, clients_size, 5);

		for(uint i = 0; i < clients_size; i++) {
			if(clients[i].active && (pollfds[i].revents & POLLIN) && (pollfds[i].revents & POLLOUT)) {
				// receive dataframe
				unsigned char header[2];
				unsigned char mask_key[4];
				uint64_t payload_length;
				unsigned char opcode;

				recv(clients[i].socket_fd, &header, 2, 0);
				opcode = header[0] & 127;
				payload_length = header[1] & 127;

//				printf("Client: %u\n", i);
//				printf("	Opcode: %x\n", opcode);

				if(opcode == 0x8) {
					close(clients[i].socket_fd);
					clients_count -= 1;
					clients[i].active = 0;
					printf("	Disconnected\n");
				}
				else {
					switch(payload_length) {
						case 126:
							payload_length = 0;
							recv(clients[i].socket_fd, &payload_length, 2, 0);
							payload_length = ntohs(payload_length);
							break;
						case 127:
							payload_length = 0;
							recv(clients[i].socket_fd, &payload_length, 8, 0);
							payload_length = be64toh(payload_length);
							break;
					}

					if(payload_length >= payload_size) {
						payload_size += payload_length;
						payload = realloc(payload, payload_size);
					}

					recv(clients[i].socket_fd, &mask_key, 4, 0);
					recv(clients[i].socket_fd, payload, payload_length * sizeof(unsigned char), 0);

					for(uint64_t j = 0; j < payload_length; j++)
						payload[j] = payload[j] ^ mask_key[j % 4];

					// handle data
					//printf("Payload: \n");
//					for(int j = 0; j < payload_length; j++)
//						printf("	%i\n", payload[j]);
//					printf("\n");
//
//					int test = 1;
//					char *p = (char*)&test;
//
//					printf("big endian? %i\n", p[0] == 0);
//
//
//					float yaw = payload[2];
////					printf("%f, %f\n\n", *((float*) &payload[2]), *((float*) &payload[6]));
//
//					printf("payload length: %u\n", payload_length);
//					printf("input size: %u\n", clients[i].frame->input_size);
//					printf("\n\n\n");

					//assert(payload_length - 2 == clients[i].frame->input_size);

					unsigned char input_type = payload[0];
					unsigned char input_index = payload[1];
					unsigned char current_input_index = 0;
					unsigned int current_byte = 0;
					//printf("input type: %u\n", input_type);

					for(unsigned int j = 0; j < clients[i].frame->input_count; j++) {
						// current type matches type
						if(clients[i].frame->inputs[j].type == input_type) {
							if(input_index == current_input_index) {
								unsigned int input_size = clients[i].frame->inputs[j].size;

								memcpy(&clients[i].input_data[current_byte], &payload[2], input_size);
								break;
							}
							current_input_index++;
						}
						current_byte += clients[i].frame->inputs[j].size;
					}
				}
			}
		}
		
		// TODO: make sleep time customizable
		nanosleep(&delay, &remaining); // sleep for 20ms, or CPU will explode
	}

	for(int i = 0; i < clients_size; i++)
		if(clients[i].active)
			close(clients[i].socket_fd);

	if(payload != NULL) {
		free(payload);
		payload = NULL;
	}
	return 0;
}

void* ma_client_accept_loop(void* data) {
	while(server_running) {
		while(clients_count < clients_size) {
			int client_fd;
			struct sockaddr_in client_address;
			socklen_t client_address_length = sizeof(client_address);

			client_fd = accept(server_fd, (struct sockaddr*) &client_address, &client_address_length);
			//printf("client found!\n");

			// Websocket handshake
			//printf("handshake initiated!\n");

			char request[500];

			recv(client_fd, &request, 500, 0);

			//printf("REQUEST:\n");
			//printf("%s\n", request);
			char upgrade_accept[] = "Upgrade: websocket\0";
			char* res = strstr(request, upgrade_accept);

			if(!res) {
				// Not a websocket connection
				send(client_fd, "Sorry lad, websockets ONLY!", 26, MSG_NOSIGNAL);
				printf("handshake failed\n");
				close(client_fd);
			}
			else if(clients_count >= clients_size) {
				printf("reached max clients\n");
				send(client_fd, "Sorry lad, max clients connected", 32, MSG_NOSIGNAL);
				close(client_fd);
			}
			else {
				// Find key in request
				char request_key_accept[] = "Sec-WebSocket-Key: ";
				char* request_key_loc = strstr(request, request_key_accept) + 19;

				char request_key[25];

				strncpy(request_key, request_key_loc, 24);
				request_key[24] = '\0';

				// Find response key
				char response_key[29];
				char magic_string[60];
				strcpy(magic_string, request_key);
				strcat(magic_string, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

				unsigned char hash[20];
				SHA1((const unsigned char*) magic_string, 60, hash);

				hash_to_base64(hash, response_key);
				response_key[28] = '\0';

				// Format HTTP response
				char response[137]; 
				bzero(response, sizeof(response));
				strcat(response, "HTTP/1.1 101 Switching Protocols\r\n");
				strcat(response, "Upgrade: websocket\r\n");
				strcat(response, "Connection: Upgrade\r\n");
				strcat(response, "Sec-WebSocket-Accept: ");
				strcat(response, response_key);
				strcat(response, "\r\n");
				strcat(response, "\r\n");

				send(client_fd, response, 129, MSG_NOSIGNAL);

				for(int i = 0; i < clients_size; i++) {
					if(clients[i].active == 0) {
						clients[i].active = 1;
						clients[i].socket_fd = client_fd;

						ma_frame_send(clients[i].frame, i);
						break;
					}
				}

				clients_count++;
			}
		}
	}

	return 0;
}

void ma_init(unsigned int max_clients, unsigned short port) {
	server_running = 1;

	// set up clients
	clients = (Client*) malloc(max_clients * sizeof(Client));

	bzero(clients, max_clients * sizeof(Client));
	for(unsigned int i = 0; i < max_clients; i++) {
		clients[i].frame = NULL;
		clients[i].input_data = NULL;
	}

	clients_count = 0;
	clients_size = max_clients;

	printf("Launching client handler\n");
	// launch client handler
	pthread_create(&client_thread, NULL, ma_client_handler, NULL);

	// launch server
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	int opt = 1;

	// make port reusable
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

	// bind
	if(bind(server_fd, (struct sockaddr*) &address, sizeof(address)) != 0) {
		perror("failed to bind to port");
		exit(1);
	}
	
	// listen
	if(listen(server_fd, max_clients) != 0) {
		perror("failed to listen to port");
		exit(1);
	}

	printf("Started server, listening on port %i\n", port);

	// start accept loop
	pthread_t accept_loop;
	pthread_create(&accept_loop, NULL, ma_client_accept_loop, NULL);
}

void ma_free() {
	server_running = 0;
	pthread_join(client_thread, NULL);

	for(int i = 0; i < clients_count; i++)
		if(clients[i].input_data != NULL)
			free(clients[i].input_data);
	free(clients);
}

void ma_send(int socket, void* data, unsigned long size) {
	unsigned char header[2];
	header[0] = 0x82;

	// assumes message is never fragmented

	unsigned char payload[size];

	if(size < 126) {
		header[1] = size;
	}
	else if(size < 0xffff) {
		header[1] = 126;
	}
	else {
		header[1] = 127;
	}

	memcpy(payload, data, size);
	send(socket, header, 2, MSG_NOSIGNAL);

	if(size < 0xffff) {
		unsigned short size_as_int = htobe16((unsigned short) size);
		send(socket, &size_as_int, sizeof(short), MSG_NOSIGNAL);
	}
	else
		send(socket, &size, sizeof(long), MSG_NOSIGNAL);

	send(socket, payload, size, MSG_NOSIGNAL);
}

// Frame binary file (very efficient)
//
// |--------------------------HEADER----------------------------------------------------------------------------------|---------INPUT-0-0-------|---INPUT-N---|--repeat for elem->
// [[type_header]-[frame_type]-[frame_orientation]-[frame_resizeable]-[frame_scrollable]-[input_count]-[element_count] [input_type]-[input-size] [    ...    ]]
//	4 bits,	   	  1 bit,	   1 bit,			   1 bit,	 	 	  1 bit			 	 1 bytes,	   1 bytes			 1 bytes,	  4 bytes.	
//
// |------------------------------------------------------------------------------------------------------------------|-------------------------|-------------|

// TODO: option to remove asserts at compile time
void ma_frame_send(Frame* frame, unsigned int client_index) {
	assert(frame != NULL || default_frame != NULL);
	assert(client_index < clients_size);

	if(frame == NULL)
		frame = default_frame;

	// assign frame to client
	// TODO: make sure this doesn't interfere with anything
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

	// header
	frame_data[0] = 0;
	frame_data[0] |= MESSAGE_FRAME		<< 4;
	frame_data[0] |= frame->type 		<< 3;
	frame_data[0] |= frame->orientation << 2;
	frame_data[0] |= frame->resizeable  << 1;
	frame_data[0] |= frame->scrollable;

	input_count   = frame->input_count;
	element_count = frame->element_count;

	memcpy(&frame_data[1], &input_count, sizeof(char));
	memcpy(&frame_data[2], &element_count, sizeof(char));
	memcpy(&frame_data[3], frame->raw_data, frame->raw_data_size);

	// send file
	printf("frame size: %u\n", frame_size);
	ma_send(clients[client_index].socket_fd, frame_data, frame_size);
	free(frame_data);
}

void ma_fetch(unsigned int client_index) {
	unsigned char header = 0;
	header = MESSAGE_INPUT_REQUEST << 4;
	ma_send(clients[client_index].socket_fd, &header, 1);
}

void ma_frame_default(Frame* frame) {
	default_frame = frame;
}

Frame* ma_frame_read(int fd) {
	unsigned char type;
	unsigned char orientation;
	read(fd, &type, 1);
	read(fd, &orientation, 1);

	unsigned int size;
	read(fd, &size, 4);
	size = htobe32(size);

	Frame* frame = malloc(sizeof(Frame) + size);
	frame->type = type;
	frame->orientation = orientation;

	frame->input_count = size;
	frame->input_size = size;

	for(int i = 0; i < size; i++) {
		unsigned char type;
		unsigned int size;
		read(fd, &type, 1);
		read(fd, &size, sizeof(unsigned int));

		frame->inputs[i].type = type;
		frame->inputs[i].size = htobe32(size);
	}

	return frame;
}

void ma_poll() {
	unsigned long totama_client_size = 0;
	for(unsigned int i = 0; i < clients_size; i++) {
		Frame* frame = clients[i].frame;

		if(frame != NULL)
			totama_client_size += frame->input_size;
	}

	if(totama_client_size > clients_data_size) {
		clients_data = realloc(clients_data, totama_client_size);
		clients_data_size = totama_client_size;
	}

	bzero(clients_data, clients_data_size);

	unsigned long current_byte = 0;
	for(unsigned int i = 0; i < clients_size; i++) {
		Frame* frame = clients[i].frame;
		if(frame != NULL) {
			if(clients[i].input_data != NULL)
				memcpy(clients_data + current_byte, &clients[i].input_data[0], frame->input_size);

			current_byte += frame->input_size;
		}
	}
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

	frame_copy->inputs = malloc(frame->input_count * sizeof(Input));
	memcpy(frame_copy->inputs, frame->inputs, frame->input_count * sizeof(Input));

	frame_copy->elements = malloc(frame->element_count * sizeof(Element));
	memcpy(frame_copy->elements, frame->elements, frame->element_count * sizeof(Element));

	frame_copy->element_data = malloc(frame->element_allocated);
	memcpy(frame_copy->element_data, frame->element_data, frame->element_size);

	frame_copy->raw_data = malloc(frame->raw_data_allocated);
	memcpy(frame_copy->raw_data, frame->raw_data, frame->raw_data_size);

	ma_frame_print(frame);
	ma_frame_print(frame_copy);

	return frame_copy;
}

// When frame is destroyed, all clients are kicked back into the default frame
void ma_frame_destroy(Frame* frame) {
	if(default_frame == frame)
		default_frame = NULL;

	for(int i = 0; i < clients_size; i++) {
		if(clients[i].frame == frame) {
			clients[i].frame = default_frame;
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

					// move existing data forward / backward
					//unsigned int bytes_left = frame->raw_data_size - (byte + original_size);
					//memmove(frame->raw_data + 5 + byte + original_size, (frame->raw_data + 5 + byte) + (element.size - original_size), bytes_left);

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


int ma_client_active(unsigned int client_index) {
	assert(client_index < clients_size);
	return clients[client_index].active;
}
