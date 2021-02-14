#define _LIBRARY_INTERNAL_
#include "library.h"
#include <pthread.h>

typedef enum {
	MESSAGE_FRAME,
	MESSAGE_INPUT,
	MESSAGE_MEDIA
} MessageType;

struct _Frame {
	FrameType type;
	Orientation orientation;
	unsigned int input_count;
	unsigned int input_size;
	Input inputs[];
};

typedef struct {
	int socket_fd; // socket and stuff
	unsigned char active; // whether a client is connected or not
	void* id; // id should be generated from ip info and stuff, so that when someone rejoins they are put on the same spot
	Frame* frame; // current frame shown
	unsigned char* input_data; // current values for each input
} Client;

// Randomly necessary for websocket handshake
void hash_to_base64(unsigned char* data, char* output);

void* l_client_handler(void* data);
void* l_client_accept_loop(void* data);
void l_send(int socket, void* data, unsigned long size); // sends a websocket

extern Client* clients;
extern unsigned int clients_count;
extern unsigned int clients_size;
extern unsigned char* clients_data; // not meant to be easily read, use l_input_get to retrieve data
extern unsigned int clients_data_size;

extern Frame* default_frame;
extern void* frames;
