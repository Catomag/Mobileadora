**WARNING THIS IS README IS STILL UNFINISHED, YOU ARE ENTERING THE DANGERZONE**

Mobileadora is a tool to use mobile clients as controllers. The library handles multiple clients and allows the user to request any data input at any time. 

# Overview
The library connects to browsers and uses websockets to communicate with clients. The server uses websockets to ask the client for information periodically. In order to function, the client needs to be able to read and understand the server's messages to generate input. Lucky for you, there is a [sister project](https://github.com/Catomag/MobileadoraClient) to this repository which implements the specification. All you need to do is download that source code, host it on an http server and profit.

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

For clients to receive a frame the `ma_frame_send` function is used, which sets the current frame of the client which will automatically be sent to them


## Important info
- when referencing an input or an element by index, it's the index of the input type. So, if I call `ma_client_input_joystick_get(..)` with `input_index = 3`, the library will look for the fourth joystick not the fourth input

---

# Using the library

## Basic usage
```
ma_init(...); // initialize the library

// do stuff with library

ma_free();
```



## Examples
I made a few examples for you to check out.

### snakes.c
Classic snake game with multiplayer added! Players get a random snake and do all the typical stuff, eat apples to become larger, dodge walls, and kick ass.

### asteroids.c
Game of asteroids, which is largely incomplete, players get points for shooting at other players and a crown is involved

### typingraces.c
A game of typeracer on a very enticing text


## Notes (For me)
- need to address endianess
- maybe turn this into a larger input library called "adora" and then use this as the mobile/browser component
