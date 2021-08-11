Mobileadora is a tool to use mobile clients as controllers. The library handles multiple clients and allows the user to request any data input at any time. 

# Overview
The library connects to browsers and uses websockets to communicate with clients. The server uses websockets to ask the client for information periodically. In order to function, the client needs to be able to read and understand the server's messages to generate input. Lucky for you, there is a [sister project](https://github.com/InspiredGoat/MobileadoraClient) to this repository which implements the specification. All you need to do is download that source code, host it on an http server and profit.

The library is made of two parts:

- Frames
	- Blueprints for the client to interpret
	- Store inputs and elements
	- Store orientation info

- Clients
	- Users that connect to the library
	- Store a reference to a frame
	- Store the input information (ie the result from a text input)


## Frames
Frames are the "pages" that the client interacts with. Frames store all input types, the overall layout of the controller and any additional elements passed to the client (A header element, some text, etc).


Frames must be created and destroyed with `ma_frame_create` and `frame_destroy`
Frames can be copied with `ma_frame_copy`, use this when you want each client to have a unique frame


Frames can be made static or dynamic, dynamic frames send info to the server immediately while static frames require a call to `ma_fetch` to send input

To populate a frame, the `ma_frame_input_*_add` and the `ma_frame_element_*_add` are used 

For clients to receive a frame the `ma_frame_send` function is used, which sets the current frame of the client. If the client is connected, they will instantly be sent that frame. If they are not connected, that frame will be sent to them as soon as they connect. This way you can have something like:
```
for(int i = 0; i < ma_clients_max_count(); i++) {
	ma_frame_send(frame, i);
}
```
and you can be sure that every client that will join will receive that frame, no matter if they are online or not.

`ma_frame_default` has a similar effect, except it won't redirect frames that are already connected, which is useful for hosting an "entry page" type thing.


## Clients
Clients store the connection information, the input data in binary as well as the current frame they are using. These things are largely abstracted though. In this library you interact with clients, by sending frames and extracting input from them.

The `ma_client_input_*_get` functions allow you to retrieve input from a particular client. The return value is a boolean which is false when the input element couldn't be found and true otherwise. The last parameter is a pointer which will get filled with the client's input information.

`ma_client_active` can be used to tell whether a client is currently connected to the server or not.

`ma_client_active_count` and `ma_clients_max_count` can be used to determine the amount of clients connected and maximum number respectively.


## Important info
- when referencing an input or an element by index, it's the index of the input type. So, if I call `ma_client_input_joystick_get(..)` with `input_index = 3`, the library will look for the fourth joystick not the fourth input

---

# Using the library

## Basic usage

The `ma_init` function starts off 2 threads which control the entire library, it must always be called first.
The `ma_free` function quits all threads and frees all library stuff. (apart from frames)
```
ma_init(...); // initialize the library

// do stuff with library

ma_free();
```

A more complete example, ask for people's names and print them
```
ma_init(10, 8080) // start a library with 10 clients maximum on port 8080

Frame* frame = ma_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL, false, false); // create a vertical frame, which only sends 

// add elements and inputs
ma_frame_element_text_add(frame, "What is your name");
ma_frame_input_text_add(frame, 15);
ma_frame_input_submit_add(frame);

ma_frame_default(frame); // any client that joins will be sent this frame

sleep(200); // wait 200 seconds

ma_fetch(); // gets input from all active clients, regardless of whether they pressed submit or not

// print all the names of the people that responded
for(int i = 0; i < ma_clients_max_count(); i++) {
	if(ma_client_active(i)) {
		char name_buffer[15];
		ma_client_input_text_get(i, 0, name_buffer); // get the ith client's 0th text input and fill the buffer with that data
		printf("%ith client's name is: %s\n", name_buffer);
	}
}

ma_frame_destroy(frame);
ma_free();
```



## Longer Examples
I made a few examples for you to check out. They all depend on [Raylib](https://www.raylib.com/), which is a pretty cool library you should check out AND by default they are all hosted on port 8000.

### snakes.c
Classic snake game with multiplayer added! Players get a random snake and do all the typical stuff, eat apples to become larger, dodge walls, and kick ass. (The code here is way prettier than the other demos)

### asteroids.c
Game of asteroids, which is largely incomplete, players get points for shooting at other players and a crown is involved

### typingraces.c
A game of typeracer on a very enticing text

### more soon maybe
