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

static int server_fd;

Client* clients;
pthread_t client_thread;
unsigned int clients_count;
unsigned int clients_size;
unsigned char* clients_data = NULL;

unsigned char server_running = 0;

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
			if(index > 64)
				printf("biggest value: %i, at %i, %i\n", index, current_byte, current_bit);

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
void* l_client_handler(void* data) {
	struct timespec delay;
	delay.tv_nsec = 10 * 1000000; // TODO: remember, 10ms of artificial delay
	delay.tv_sec = 0;
	struct timespec remaining;

	unsigned int payload_size = 200;
	unsigned char* payload = malloc(sizeof(unsigned char) * payload_size);

	while(server_running) {
		struct pollfd pollfds[clients_count];
		for(uint i = 0; i < clients_count; i++) {
			pollfds[i].fd = clients[i].socket_fd;
			pollfds[i].events = POLLIN | POLLOUT;
		}
		poll(pollfds, clients_count, 10);

		for(uint i = 0; i < clients_count; i++) {
			if(clients[i].active && (pollfds[i].revents & POLLIN) && (pollfds[i].revents & POLLOUT)) {
				// receive dataframe
				unsigned char header[2];
				unsigned char mask_key[4];
				uint64_t payload_length;
				unsigned char opcode;

				recv(clients[i].socket_fd, &header, 2, 0);
				opcode = header[0] & 127;
				payload_length = header[1] & 127;

				printf("Client: %u\n", i);
				printf("	Opcode: %x\n", opcode);

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
						payload_size += payload_length + 1;
						payload = realloc(payload, payload_size);
					}

					recv(clients[i].socket_fd, &mask_key, 4, 0);
					recv(clients[i].socket_fd, payload, payload_length * sizeof(unsigned char), 0);

					for(uint64_t i = 0; i < payload_length; i++)
						payload[i] = payload[i] ^ mask_key[i % 4];

					payload[payload_length] = '\0';
					// handle data
					printf("	Payload: ");
					for(int i = 0; i < payload_length; i++)
						printf("	%i\n", payload[i]);
					printf("\n");

					// TODO: decipher payload into distinct elements
					// 1. Read 1 byte for input type, read 1 byte for input index
					// 2. Find frame structure for client
					// 3. Find input index and test if it is valid
					// 4. Add new input to the client's data thingo

					// TODO: populate clients array with data
					//clients[i].input_data[]
				}
			}
		}
		
		// TODO: make sleep time customizable
		nanosleep(&delay, &remaining); // sleep for 20ms, or CPU will explode
	}

	for(int i = 0; i < clients_count; i++)
		if(clients[i].active)
			close(clients[i].socket_fd);

	free(payload);
	return 0;
}

void* l_client_accept_loop(void* data) {
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
				char request_key_accept[] = "Sec-WebSocket-Key: ";
				char* request_key_loc = strstr(request, request_key_accept) + 19;

				char request_key[25];

				strncpy(request_key, request_key_loc, 24);
				request_key[24] = '\0';

				char response_key[29];
				char magic_string[60];
				strcpy(magic_string, request_key);
				strcat(magic_string, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

				unsigned char hash[20];
				SHA1((const unsigned char*) magic_string, 60, hash);

				hash_to_base64(hash, response_key);
				response_key[28] = '\0';

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

				printf("handshake completed!\n");

				Client client;
				client.active = 1; // setting to active will indirectly allow thread to start
				client.socket_fd = client_fd;
				for(int i = 0; i < clients_size; i++) {
					if(clients[i].active == 0) {
						clients[i] = client;
						if(default_frame != NULL)
							l_frame_send(default_frame, i);
						break;
					}
				}


				clients_count++;
			}
		}
	}

	return 0;
}

void l_init(unsigned int max_clients, unsigned short port) {
	server_running = 1;

	// set up clients
	clients = (Client*) malloc(max_clients * sizeof(Client));

	bzero(clients, max_clients * sizeof(Client));
	clients_count = 0;
	clients_size = max_clients;

	// launch client handler
	pthread_create(&client_thread, NULL, l_client_handler, NULL);

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
	pthread_create(&accept_loop, NULL, l_client_accept_loop, NULL);
}

void l_free() {
	server_running = 0;
}

void l_send(int socket, void* data, unsigned long size) {
	unsigned char header[2];
	header[0] = 0x80;
	header[0] = header[0] | 2;

	// assumes message is never fragmented

	uint64_t payload_length_size = 0;
	payload_length_size += (size > 125 && size <= 0xffff) * 0xffff;
	payload_length_size += (size > 0xffff && size <= 0xffffffffffffffff) * 0xffffffffffffffff;

	unsigned char payload[payload_length_size + size];

	if(size < 126) {
			header[1] = size;
	}
	else if(size < 65536) {
			header[1] = 126;
			payload[0] = size;
			payload_length_size = 2;
	}
	else {
			header[1] = 127;
			payload[0] = size;
			payload_length_size = 8;
	}

	for(uint64_t i = 0; i < size; i++)
			payload[payload_length_size + i] = (unsigned char) ((unsigned char*)data)[i];

	uint64_t payload_length_buf = 0;
	payload_length_buf = htobe64(size);

	send(socket, header, 2, MSG_NOSIGNAL);
	send(socket, &payload_length_buf, payload_length_size, MSG_NOSIGNAL);
	send(socket, payload, size, MSG_NOSIGNAL);
}

// Frame binary file (very efficient)
//
// |--------------------------HEADER----------------------------|---------INPUT-0-0-------|---INPUT-N---|
// [[type_header]-[frame_type]-[frame_orientation]-[input_count] [input_type]-[input-size] [    ...    ]]
//	1 byte,	   	  1 byte,	   1 byte,			   4 bytes,	 	 1 byte,	  4 bytes,	   5 bytes.	
// |------------------------------------------------------------|-------------------------|-------------|

<<<<<<< HEAD
// TODO: option to remove asserts at compile time
void l_frame_send(Frame* frame, unsigned int client_index) {
	assert(default_frame != NULL);
=======
// TODO: remove asserts at compile time
void l_frame_send(Frame* frame, unsigned int client_index) {
	assert(frame != NULL);
>>>>>>> c8fddc4c925d47e7ddcfbd0846c995734548b9b3
	assert(client_index < clients_size);

	if(frame == NULL)
		frame = default_frame;

	// assign frame to client
	// TODO: make sure this doesn't interfere with anything
	if(clients[client_index].active) {
		if(clients[client_index].frame->input_size < frame->input_size)
			clients[client_index].input_data = realloc(clients[client_index].input_data, frame->input_size);
	}
	clients[client_index].frame = frame;
	
	// construct file

	unsigned int frame_size = 7 + (frame->input_count * (5) + frame->input_size);
	unsigned char* frame_data = malloc(frame_size);

	// header
	frame_data[0] = MESSAGE_FRAME;
	frame_data[1] = frame->type;
	frame_data[2] = frame->orientation;
	*((unsigned int*) &frame_data[3]) = htobe32(frame->input_count);

	// input types
	unsigned int byte_index = 7;
	for(unsigned int i = 0; i < frame->input_count; i++) {
		frame_data[byte_index] = frame->inputs[i].type;
		byte_index++;

		*((unsigned int*) &frame_data[byte_index]) = frame->inputs[i].size;
		byte_index += 4;
	}
	
	// send file
<<<<<<< HEAD
	l_send(clients[client_index].socket_fd, frame_data, frame_size );
}

void l_default(Frame* frame) {
	default_frame = frame;
=======
	l_send(clients[client_index].socket_fd, frame_data, frame_size, SOCK_NONBLOCK);
	free(frame);
}

Frame* l_frame_read(int fd) {
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
>>>>>>> c8fddc4c925d47e7ddcfbd0846c995734548b9b3
}

void l_poll() {
	unsigned long total_client_size = 0;
	for(unsigned int i = 0; i < clients_size; i++) {
		Frame* frame = clients[i].frame;
		if(frame == NULL)
			frame = default_frame;

		total_client_size += frame->input_size;
	}

	if(total_client_size > clients_data_size)
		clients_data = realloc(clients_data, total_client_size);


	unsigned long current_byte;
	for(unsigned int i = 0; i < clients_size; i++) {
		Frame* frame = clients[i].frame;
		if(frame == NULL)
			frame = default_frame;
		
		for(unsigned int j = 0; j < frame->input_size; j++)
			clients_data[current_byte + j] = clients[i].input_data[j];

		current_byte += frame->input_size;
	}
}



Frame* l_frame_create(FrameType type, Orientation orientation) {
	Frame frame;
	frame.type = type;
	frame.orientation = orientation;
	frame.input_size = 0;
	frame.input_count = 0;
	return malloc(sizeof(Frame));
}

void l_frame_destroy(Frame* frame) {
	free(frame);
	for(unsigned int i = 0; i < clients_size; i++) {
		pthread_join(client_thread, NULL);
	}
}

// TODO: verify this works
void l_frame_input_add(Frame* frame, Input input) {
	frame = realloc(frame, sizeof(Frame) + sizeof(Input) * (frame->input_count + 1)); // this might be expensive
	frame->inputs[frame->input_count].type = input.type;
	frame->inputs[frame->input_count].size = input.size;

	unsigned int total_input_size = 0;
	for(unsigned int i = 0; i < frame->input_count; i++) {
		total_input_size += frame->inputs[i].size;
	}

	frame->input_count++;
	frame->input_size += input.size;
}

void l_frame_print(Frame* frame) {
	printf("orientation: %s\n", (frame->orientation == ORIENTATION_VERTICAL) ? "vertical" : "horizontal");
	printf("type: %s\n", (frame->type == FRAME_STATIC) ? "static" : "dynamic");

	for(unsigned int i = 0; i < frame->input_count; i++) {
		printf("	%i Input: %i, length: %u\n", i, frame->inputs[i].type, frame->inputs[i].size);
	}

	printf("input size: %u\n", frame->input_size);
	printf("input count: %u\n", frame->input_count);
}


