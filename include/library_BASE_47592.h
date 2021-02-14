
// type declarations
typedef struct _Frame Frame;

#define NULL_CLIENT ((unsigned int) -1)

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

extern void l_send(Frame* frame, unsigned int client_index); // sends frame to client on specified index
extern void l_default(Frame* frame); // set frame as default
extern void l_poll(); // retrieves client information and current input information

extern unsigned int l_client_index_from_id(void* client_id); // returns NULL_CLIENT if id is invalid or client is disconnected
extern void* l_client_id_from_index(unsigned int client_index); // returns NULL_CLIENT if id is invalid or client is disconnected
extern int l_client_active(unsigned int client_index); // checks if the client is connected
extern unsigned int l_client_active_count(); // returns number of active clients
extern unsigned int l_client_max_count(); // returns number of maximum clients

extern Frame* l_frame_create(FrameType type, Orientation orientation);
extern Frame* l_frame_copy(Frame* frame); // create copy of existing frame, copy must be freed with frame_destroy
extern void l_frame_destroy(Frame* frame);

extern void l_frame_client_count(Frame* frame);
extern void l_frame_fetch(Frame* frame); // manually asks clients for data, used in static frames
extern void l_frame_print(Frame* frame); // print contents of frame object

extern void l_frame_text_add(Frame* frame, const char* text); // request some text be added to the frame
extern void l_frame_text_set(Frame* frame, unsigned int index, const char* text); // change the text
extern void l_frame_input_add(Frame* frame, Input input); // add input to the frame

extern Input l_input_text_create(unsigned short max_chars);
extern Input l_input_button_create(const char* text);
extern Input l_input_submit_create(const char* text);
extern Input l_input_toggle_create(const char* text);
extern Input l_input_joystick_create();
extern Input l_input_generic_create(unsigned int size);

extern int l_input_get_all(Input type, unsigned char index, void* data); // returns all input data from all clients for a specific input
extern int l_input_get(unsigned int client_index, Input type, unsigned char index, void* data);
extern int l_input_text_get(unsigned int client_index, unsigned char input_index, char* value);
extern int l_input_button_get(unsigned int client_index, unsigned char input_index, unsigned char* value);
extern int l_input_submit_get(unsigned int client_index, unsigned char input_index, unsigned char* value);
extern int l_input_toggle_get(unsigned int client_index, unsigned char input_index, unsigned char* value);
extern int l_input_joystick_get(unsigned int client_index, unsigned char input_index, float* x_value, float* y_value);








