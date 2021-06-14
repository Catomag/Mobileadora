#define _LIBRARY_INTERNAL_
#include "library.h"
#include <pthread.h>

// max 16
typedef enum {
	MESSAGE_FRAME,
	MESSAGE_INPUT_REQUEST,
} MessageType;

typedef enum {
	INPUT_TEXT,
	INPUT_BUTTON, // 1 when held, 0 when not
	INPUT_BUTTON_SEND, // when pressed, sends frame to server
	INPUT_TOGGLE, // toggles between on and off every press
	INPUT_JOYSTICK,	// 2 float array
	INPUT_GENERIC,	// byte array
} InputType;

struct _Input {
	unsigned char type;
	unsigned int size; // size in bytes
};

typedef enum {
	ELEMENT_TEXT,
	ELEMENT_BREAK, // <br> in html
	ELEMENT_SPACER, // A space ~size of a button
	ELEMENT_HEADER1,
	ELEMENT_HEADER2,
	ELEMENT_HEADER3,
	ELEMENT_LINE, // a horizontal line, html style
	ELEMENT_COLOR, // a little coloured square
} ElementType;

struct _Element {
	unsigned char type;
	unsigned int size; // size can be 0
};

struct _Frame {
	bool type;
	bool orientation;
	bool scrollable;
	bool resizeable;
	unsigned char input_count;
	unsigned char element_count;
	unsigned int input_size;
	unsigned int element_size;
	unsigned int element_allocated;
	Input* inputs;
	Element* elements; // element info
	void* element_data; // data per element

	void* raw_data; // all of the input and element data ordered sequentially, importantly element types in this array are stored as bytes with the first bit enabled so that any client can easily work out what types it has
	unsigned long raw_data_size; // the raw data array could potentially be quite large, so we prepare accordingly
	unsigned long raw_data_allocated;
};

typedef struct {
	int socket_fd; // socket and stuff
	unsigned char active; // whether a client is connected or not
	void* id; // id should be generated from ip info and stuff, so that when someone rejoins they are put on the same spot
	Frame* frame; // current frame shown
	unsigned char* input_data; // current values for each input
} Client;




// Randomly necessary for websocket handshake
extern void hash_to_base64(unsigned char* data, char* output);

extern void ma_send(int socket, void* data, unsigned long size); // sends a websocket

extern void* ma_client_accept_loop(void* data); // accepts clients attempting to connect
extern void* ma_client_handler(void* data); // handles clients after connecting

extern void ma_frame_input_add(Frame* frame, Input input); // add input to the frame
extern void ma_frame_element_add(Frame* frame, Element element, void* data); // adds an element to the frame
extern void ma_frame_element_set(Frame* frame, Element element, unsigned char index, void* data); // changes the data of an element at given index

extern bool ma_input_get(unsigned int client_index, InputType type, unsigned char index, void* data);


extern Client* clients;
extern unsigned int clients_count;
extern unsigned int clients_size;

extern Frame* default_frame;
extern void* frames;
