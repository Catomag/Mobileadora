
// type declarations
typedef struct _Frame Frame;

typedef enum {
	INPUT_TEXT,
	INPUT_BUTTON,
	INPUT_TOGGLE,
	INPUT_VECTOR,	// n float array
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

// function declarations

// client_id, input_type, input_index, data

extern void library_init(unsigned int max_clients, float heartbeat_rate); // initializes all services pertaining to the library
extern void library_free();

extern Frame* library_frame_create();
extern void library_frame_destroy(Frame* frame);

extern void library_frame_send(Frame* frame);
extern void library_frame_retrieve(Frame* frame);
extern void library_frame_print(Frame* frame);

extern void library_frame_input_add(Frame* frame, Input input);
extern void library_frame_text_add(Frame* frame, const char* text);
extern void library_frame_orientation_set(Orientation orientation);

extern Input library_input_text_create(unsigned short max_chars);
extern Input library_input_button_create(const char* text);
extern Input library_input_toggle_create(const char* text);
extern Input library_input_vector_create(unsigned int dimensions);
extern Input library_input_generic_create(unsigned int size);

extern float* library_input_vector_retrieve();
