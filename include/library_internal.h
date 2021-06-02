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
	INPUT_COUNT
} InputType;

struct _Input {
	InputType type;
	unsigned int size; // size in bytes
};

typedef enum {
	ELEMENT_TEXT,
	ELEMENT_BREAK, // <br> in html
	ELEMENT_LINE,
	ELEMENT_HEADER1,
	ELEMENT_HEADER2,
	ELEMENT_HEADER3,
	ELEMENT_COLOR, // a little colored square
	ELEMENT_IMAGE, // image url
} ElementType;

struct _Element {
	ElementType type;
	unsigned int size; // size can be 0
};

struct _Frame {
	FrameType type;
	Orientation orientation;
	bool scrollable;
	bool resizeable;
	unsigned char input_count;
	unsigned int input_size;
	unsigned char element_count;
	unsigned int element_size;
	unsigned int element_allocated;
	Element* elements; // element info
	void* element_data; // data per element
	// inputs are allocated with frame struct to reduce cache misses, since input data is often accessed with frame data
	Input* inputs;
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

extern void* ma_client_handler(void* data);
extern void* ma_client_accept_loop(void* data);

extern void ma_frame_input_add(Frame* frame, Input input); // add input to the frame
extern void ma_frame_element_add(Frame* frame, Element element, void* data); // adds an element to the frame
extern void ma_frame_element_set(Frame* frame, Element element, unsigned char index, void* data); // changes the data of an element at given index

extern bool ma_input_get(unsigned int client_index, InputType type, unsigned char index, void* data);


extern Client* clients;
extern unsigned int clients_count;
extern unsigned int clients_size;
extern void* clients_data; // not meant to be read directly, use ma_input_get to retrieve data
extern unsigned long clients_data_size;

extern Frame* default_frame;
extern void* frames;
