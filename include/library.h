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

extern void l_init(unsigned int max_clients, unsigned short port); // starts server and creates separate thread to handle clients
extern void l_free(); // stops library and frees allocated resources 

extern void l_poll(); // retrieves client information and current input information
extern void l_fetch(unsigned int client_index); // manually asks clients for data, used in static frames

extern unsigned int l_client_index_from_id(void* client_id); // returns NULL_CLIENT if id is invalid or client is disconnected
extern void* l_client_id_from_index(unsigned int client_index); // returns NULL_CLIENT if id is invalid or client is disconnected
extern int l_client_active(unsigned int client_index); // checks if the client is connected
extern unsigned int l_client_active_count(); // returns number of active clients
extern unsigned int l_client_max_count(); // returns number of maximum clients

extern bool l_client_input_generic_get(unsigned int client_index, unsigned char input_index, void* value);
extern bool l_client_input_text_get(unsigned int client_index, unsigned char input_index, char* value);
extern bool l_client_input_button_get(unsigned int client_index, unsigned char input_index, bool* value);
//extern bool l_client_input_submit_get(unsigned int client_index, unsigned char input_index, bool* value); shouldn't be able to get this one
extern bool l_client_input_toggle_get(unsigned int client_index, unsigned char input_index, bool* value);
extern bool l_client_input_joystick_get(unsigned int client_index, unsigned char input_index, float* x_value, float* y_value);

extern Frame* l_frame_create(FrameType type, Orientation orientation, bool scrollable, bool resizeable);
extern void l_frame_destroy(Frame* frame);
extern Frame* l_frame_copy(Frame* frame); // create copy of existing frame, copy must be freed with frame_destroy
extern void l_frame_send(Frame* frame, unsigned int client_index); // sets frame to client on specified index

extern void l_frame_default(Frame* frame); // set frame as default
extern void l_frame_print(Frame* frame); // print contents of frame object

extern void l_frame_input_generic_add(Frame* frame, unsigned int size); // a buffer of bytes
extern void l_frame_input_text_add(Frame* frame, unsigned short max_chars);
extern void l_frame_input_button_add(Frame* frame);
extern void l_frame_input_submit_add(Frame* frame);
extern void l_frame_input_toggle_add(Frame* frame);
extern void l_frame_input_joystick_add(Frame* frame);

// BIG DILEMA, NOT SURE HOW TO DEAL WITH ELEMENTS
extern void l_frame_text_add(Frame* frame, const char* string);
extern void l_frame_h1_add(Frame* frame, const char* string);
extern void l_frame_h2_add(Frame* frame, const char* string);
extern void l_frame_h3_add(Frame* frame, const char* string);
extern void l_frame_color_add(Frame* frame, unsigned char r, unsigned char g, unsigned char b);
extern void l_frame_break_add(Frame* frame);
extern void l_frame_line_add(Frame* frame);


/*

l_element_text_add(Frame* frame, const char* string);

l_frame_text_add(Frame* frame, const char* string);
l_frame_color_add(Frame* frame, unsigned char r, unsigned char g, unsigned char b);
l_frame_color_set(Frame* frame, unsigned char input_index, unsigned char r, unsigned char g, unsigned char b);

// create different teams
---

l_init(...);
Frame* team1 = l_frame_create(...);
l_frame_text_add(team1, "You are in team 1");
l_frame_input_add(team1, l_input_joystick_create());
l_frame_input_add(team1, l_input_button_create());
l_frame_input_joystick_add(team1);

// each team gets a "unique" frame
Frame* team2 = l_frame_copy(team1);
l_frame_text_set(team2, "You are in team 2");

// wait for people to join
sleep(5);

for(int i = 0; i < l_client_active_count() / 2; i++) {
	l_frame_send(team1, i);
}

for(int i = l_client_active_count(); i > l_client_active_count() / 2; i--) {
	l_frame_send(team2, i);
}

... game

l_client_input_joystick_get(unsigned int client_index, unsigned char input_index, float* x_value, float* y_value);
l_client_input_text_get(unsigned int client_index, unsigned char input_index, unsigned char* value);


---


*/






#endif
