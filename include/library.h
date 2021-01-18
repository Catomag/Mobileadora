
// type declarations
typedef struct _Frame Frame;

typedef enum {
	FRAME_STATIC, // retrieves input when prompted
	FRAME_DYNAMIC // retrieves input automatically
} FrameType;

typedef enum {
	INPUT_TEXT,
	INPUT_BUTTON, // 1 when held, 0 when not
	INPUT_TOGGLE, // toggles between on and off every press
	INPUT_JOYSTICK,	// 2 float array
	INPUT_GENERIC,	// byte array
	INPUT_COUNT
} InputType;

typedef enum {
	ORIENTATION_VERTICAL,
	ORIENTATION_HORIZONTAL
} Orientation;

typedef struct {
	InputType type;
	unsigned int size; // size in bytes
} Input;


// Core library functions

extern void l_init(unsigned int max_clients, unsigned short port); // starts server and creates separate thread to handle clients
extern void l_free(); // stops library and frees allocated resources 
extern void l_send(Frame* frame, unsigned int client_index); // sends frame to client on specified id
extern void l_poll(); // retrieves client input information
extern unsigned int l_client_count();

extern Frame* l_frame_create(FrameType type, Orientation orientation);
extern void l_frame_destroy(Frame* frame);

extern void l_frame_client_count(Frame* frame);
extern void l_frame_fetch(Frame* frame); // manually asks clients for data, used in static frames
extern void l_frame_print(Frame* frame); // print contents of frame object

extern void l_frame_text_add(Frame* frame, const char* text);
extern void l_frame_input_add(Frame* frame, Input input);

extern Input l_input_text_create(unsigned short max_chars);
extern Input l_input_button_create(const char* text);
extern Input l_input_toggle_create(const char* text);
extern Input l_input_joystick_create();
extern Input l_input_generic_create(unsigned int size);

extern int l_input_get(unsigned int client_index, Input type, unsigned char text_index, void* data);
extern int l_input_text_get(unsigned int client_index, unsigned char input_index, char* value);
extern int l_input_button_get(unsigned int client_index, unsigned char input_index, unsigned char* value);
extern int l_input_toggle_get(unsigned int client_index, unsigned char input_index, unsigned char* value);
extern int l_input_joystick_get(unsigned int client_index, unsigned char input_index, float* value);

// Frame:
// 		- An object that is passed around, referenced as void* (SFML style)
// 			- People can store multiple frames easily
// 			struct 
// 			Frame* login = frame_create();
// 			Frame* controller = frame_create();
//			
//			Input button = input_button_create("hello");
//			frame_add_input(login, button);
//			frame_send(login);
//
//			// request update
//			frame_fetch(login);
// 			if(input_get_toggle(login, 0))
// 				frame_send(controller);
// 		- bind frames
// 			- all clients get the same frame


// Options:
// 		- get void* and size for each object and then user casts it
// 			// read 2nd joystick
// 			float res[2] = (float*) frame_input_read(frame, JOYSTICK, 1);
// 		- have a custom method for each input
// 			// read 2nd joystick
// 			float res[2];
// 			frame_input_joystick_read(frame, 1, &res);
// 		- 







