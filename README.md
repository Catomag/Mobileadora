# Mobileadora

**WARNING THIS IS README IS STILL UNFINISHED, YOU ARE ENTERING THE DANGERZONE**

Mobileadora is a tool to use mobile clients as controllers. The library handles multiple clients and allows the user to request any data input at any time. 

## How it works
The library connects to browsers and uses websockets to communicate with clients. The server uses websockets to ask the client for information periodically. In order to function, the client needs to be able to read and understand the server's messages to generate input. Lucky for you, there is a [sister project](https://github.com/Catomag/MobileadoraClient) to this repository which implements the specification. All you need to do is download that source code, host it on an http server and profit.

## Style
All library functions start with *ma_*, and that's about it

## Frames
Frames are the "pages" that the client interacts with. Frames store all input types, the overall layout of the controller and any additional elements passed to the client (A header element, some text, etc).

Frames must be created and destroyed with frame_create() and frame_destroy()

## Examples
By the time you are reading this, there will hopefully be some examples for you to check out.


## Notes (For me)
- need to address endianess
- maybe turn this into a larger input library called "adora" and then use this as the mobile/browser component
