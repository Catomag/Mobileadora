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
	ORIENTATION_VERTICAL,
	ORIENTATION_HORIZONTAL
} Orientation;



// Core library functions

extern void ma_init(unsigned int max_clients, unsigned short port); // starts server and creates separate thread to handle clients
extern void ma_free(); // stops library and frees allocated resources 

extern void ma_flush(); // resets all client info (forcefully sets everything to 0)
extern void ma_fetch(); // manually asks clients for data, used in static frames

extern int ma_client_active(unsigned int client_index); // checks if the client is connected
extern void ma_client_disconnect(unsigned int client_index); // disconnects client forcefully
extern unsigned int ma_client_active_count(); // returns number of active clients
extern unsigned int ma_client_max_count(); // returns number of maximum clients

extern void ma_client_flush(unsigned int client_index); // same as ma_flush, but targets specific client
extern void ma_client_fetch(unsigned int client_index); // same as ma_flush, but targets specific client

extern bool ma_client_input_generic_get(unsigned int client_index, unsigned char input_index, void* value);
extern bool ma_client_input_text_get(unsigned int client_index, unsigned char input_index, char* value);
extern bool ma_client_input_button_get(unsigned int client_index, unsigned char input_index, bool* value);
extern bool ma_client_input_submit_get(unsigned int client_index, unsigned char input_index, bool* value);
extern bool ma_client_input_toggle_get(unsigned int client_index, unsigned char input_index, bool* value);
extern bool ma_client_input_joystick_get(unsigned int client_index, unsigned char input_index, float* x_value, float* y_value);

extern Frame* ma_frame_create(FrameType type, Orientation orientation, bool scrollable, bool resizeable);
extern void ma_frame_destroy(Frame* frame);
extern Frame* ma_frame_copy(Frame* frame); // create copy of existing frame, copy must be freed with frame_destroy
extern void ma_frame_send(Frame* frame, unsigned int client_index); // sets frame to client on specified index

extern void ma_frame_default(Frame* frame); // set frame as default
extern void ma_frame_print(Frame* frame); // print contents of frame object

// add input to the frame
extern void ma_frame_input_generic_add(Frame* frame, unsigned int size); // a buffer of bytes
extern void ma_frame_input_text_add(Frame* frame, unsigned int max_chars);
extern void ma_frame_input_button_add(Frame* frame);
extern void ma_frame_input_submit_add(Frame* frame);
extern void ma_frame_input_toggle_add(Frame* frame);
extern void ma_frame_input_joystick_add(Frame* frame);

// add elements to the frame
extern void ma_frame_element_text_add(Frame* frame, const char* string);
extern void ma_frame_element_h1_add(Frame* frame, const char* string);
extern void ma_frame_element_h2_add(Frame* frame, const char* string);
extern void ma_frame_element_h3_add(Frame* frame, const char* string);
extern void ma_frame_element_color_add(Frame* frame, unsigned char r, unsigned char g, unsigned char b);
extern void ma_frame_element_break_add(Frame* frame);
extern void ma_frame_element_spacer_add(Frame* frame);
extern void ma_frame_element_line_add(Frame* frame); // a nice markdown type line

// allow editing of current elements
extern void ma_frame_element_text_set(Frame* frame, unsigned char element_index, const char* string);
extern void ma_frame_element_h1_set(Frame* frame, unsigned char element_index, const char* string);
extern void ma_frame_element_h2_set(Frame* frame, unsigned char element_index, const char* string);
extern void ma_frame_element_h3_set(Frame* frame, unsigned char element_index, const char* string);
extern void ma_frame_element_color_set(Frame* frame, unsigned char element_index, unsigned char r, unsigned char g, unsigned char b);

#endif
