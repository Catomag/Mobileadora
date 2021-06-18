#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <poll.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>

#include <openssl/sha.h>

#include "../include/mobileadora_internal.h"




static int server_fd;

Client* clients;
pthread_t client_thread;
unsigned int clients_count;
unsigned int clients_size;

unsigned char server_running = 0;




char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
// output needs to be 28 bytes, data needs to be 20 bytes
void hash_to_base64(unsigned char* data, char* output) {
	unsigned char current_bit = 0;
	unsigned char group_count = 1;
	unsigned char index = 0;

	// go through each bit and perform the algorithm
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

	index = (data[19] & 15) << 2;
	output[26] = base64_alphabet[index];
	output[27] = '=';
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

	if(size >= 126 && size < 0xffff) {
		unsigned short size_as_int = htobe16((unsigned short) size);
		send(socket, &size_as_int, sizeof(short), MSG_NOSIGNAL);
	}
	else if(size > 0xffff)
		send(socket, &size, sizeof(long), MSG_NOSIGNAL);

	send(socket, payload, size, MSG_NOSIGNAL);
}




void ma_fetch() {
	unsigned char header = 0;
	header = MESSAGE_INPUT_REQUEST << 4;
	for(int i = 0; i < clients_size; i++)
		if(clients[i].active)
			ma_send(clients[i].socket_fd, &header, 1);
}




void ma_flush() {
	// if client has data force it to be zero
	for(int i = 0; i < clients_size; i++)
		if(clients[i].frame != NULL && clients[i].input_data != NULL)
			bzero(clients[i].input_data, clients[i].frame->input_size);
}



void ma_client_fetch(unsigned int client_index) {
	assert(client_index < clients_size);
	unsigned char header = 0;
	header = MESSAGE_INPUT_REQUEST << 4;

	if(clients[client_index].active)
		ma_send(clients[client_index].socket_fd, &header, 1);
}




void ma_client_flush(unsigned int client_index) {
	assert(client_index < clients_size);
	// if client has data force it to be zero
	if(clients[client_index].frame != NULL && clients[client_index].input_data != NULL)
		bzero(clients[client_index].input_data, clients[client_index].frame->input_size);
}




int ma_client_active(unsigned int client_index) {
	assert(client_index < clients_size);
	return clients[client_index].active;
}




unsigned int ma_client_active_count() {
	return clients_count;
}

unsigned int ma_client_max_count() {
	return clients_size;
}



void ma_client_disconnect(unsigned int client_index) {
	assert(client_index < clients_size);
	close(clients[client_index].socket_fd);
	clients_count -= 1;
	clients[client_index].active = 0;
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




void* ma_client_handler(void* data) {
	struct timespec delay;
	delay.tv_nsec = 5 * 1000000;
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


				// if the websocket has opcode 8, it's sending a disconnect signal
				if(opcode == 0x8) {
					ma_client_disconnect(i);
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
					unsigned char input_type = payload[0];
					unsigned char input_index = payload[1];
					unsigned char current_input_index = 0;
					unsigned int current_byte = 0;

					// copy memory into client's input data buffer
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
		
		nanosleep(&delay, &remaining); // sleep for a few milliseconds, or else CPU will explode
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

			// Websocket handshake

			char request[500];

			recv(client_fd, &request, 500, 0);

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

				// Format HTTP response (buffer is slightly bigger than needed)
				char response[140] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "; 
				strcat(response, response_key);
				strcat(response, "\r\n\r\n");

				send(client_fd, response, 129, MSG_NOSIGNAL);

				for(int i = 0; i < clients_size; i++) {
					if(clients[i].active == 0) {
						clients[i].active = 1;
						clients[i].socket_fd = client_fd;

						// as soon as a client connects, whatever frame the client currently has set to them is selected
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



