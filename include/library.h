#ifndef LIBRARY_H
#define LIBRARY_H

// type declarations
#ifndef bool
typedef unsigned char bool;
#endif
typedef struct _Frame Frame;
typedef struct _Input Input;
typedef struct _Element Element;

#define NULL_CLIENT ((unsigned int) -1)
#define ALL_CLIENTS ((unsigned int) -1)

typedef enum {
	FRAME_STATIC, // retrieves input when prompted
	FRAME_DYNAMIC // retrieves input automatically
} FrameType;

typedef enum {
	INPUT_TEXT,
	INPUT_BUTTON, // 1 when held, 0 when not
	INPUT_BUTTON_SEND, // when pressed, sends frame to server
	INPUT_TOGGLE, // toggles between on and off every press
	INPUT_JOYSTICK,	// 2 float array
	INPUT_GENERIC,	// byte array
	INPUT_COUNT
} InputType;

typedef enum {
	ELEMENT_TEXT,
	ELEMENT_BREAK, // <br> in html
	ELEMENT_LINE,
	ELEMENT_HEADER1,
	ELEMENT_HEADER2,
	ELEMENT_HEADER3,
	ELEMENT_IMAGE,
} ElementType;

typedef enum {
	ORIENTATION_VERTICAL,
	ORIENTATION_HORIZONTAL
} Orientation;

// TODO: move this to library_internal.h
struct _Input {
	InputType type;
	unsigned int size; // size in bytes
};

struct _Element {
	ElementType type;
	unsigned int size; // size can be 0
	unsigned char data[];
};


// Core library functions

extern void l_init(unsigned int max_clients, unsigned short port); // starts server and creates separate thread to handle clients
extern void l_free(); // stops library and frees allocated resources 

extern void l_poll(); // retrieves client information and current input information
extern void l_fetch(unsigned int client_index); // manually asks clients for data, used in static frames

extern unsigned int l_client_index_from_id(void* client_id); // returns NULL_CLIENT if id is invalid or client is disconnected
extern void* l_client_id_from_index(unsigned int client_index); // returns NULL_CLIENT if id is invalid or client is disconnected
extern int l_client_active(unsigned int client_index); // checks if the client is connected
extern unsigned int l_client_active_count(); // returns number of active clients
extern unsigned int l_client_max_count(); // returns number of maximum clients

extern Frame* l_frame_create(FrameType type, Orientation orientation, bool scrollable, bool resizeable);
extern void l_frame_destroy(Frame* frame);
extern Frame* l_frame_copy(Frame* frame); // create copy of existing frame, copy must be freed with frame_destroy
extern void l_frame_send(Frame* frame, unsigned int client_index); // sets frame to client on specified index

extern void l_frame_default(Frame* frame); // set frame as default
extern void l_frame_print(Frame* frame); // print contents of frame object

extern void l_frame_input_add(Frame* frame, Input input); // add input to the frame
extern void l_frame_element_add(Frame* frame, Element element); // adds an element to the frame

extern Input l_input_generic_create(unsigned int size);
extern Input l_input_text_create(unsigned short max_chars);
extern Input l_input_button_create();
extern Input l_input_submit_create();
extern Input l_input_toggle_create();
extern Input l_input_joystick_create();

extern Element l_element_text_create(const char* string);
extern Element l_element_h1_create(const char* string);
extern Element l_element_h2_create(const char* string);
extern Element l_element_h3_create(const char* string);
extern Element l_element_break_create();
extern Element l_element_line_create();

extern int l_input_get_all(Input type, unsigned char index, void* data); // returns all input data from all clients for a specific input
extern bool l_input_get(unsigned int client_index, InputType type, unsigned char index, unsigned char* data);
extern bool l_input_text_get(unsigned int client_index, unsigned char input_index, char* value);
extern bool l_input_button_get(unsigned int client_index, unsigned char input_index, unsigned char* value);
extern bool l_input_submit_get(unsigned int client_index, unsigned char input_index, unsigned char* value);
extern bool l_input_toggle_get(unsigned int client_index, unsigned char input_index, unsigned char* value);
extern bool l_input_joystick_get(unsigned int client_index, unsigned char input_index, float* x_value, float* y_value);





#endif
