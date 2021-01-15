# MobileConnector

MobileConnector (Better name pending) is a tool to use mobile clients as controllers. The library handles multiple clients and allows the user to request any data input at any time. 

## How it works
The library connects to browsers and uses websockets to communicate with clients. The server sends 

## Frames
Frames are the "pages" that the client interacts with. Frames store all input types and the overall layout of the controller.

Frames must be created and destroyed with frame_create() and frame_destroy()

```
int main() {
}
```

## Customization
To customize the look and size of input simply add your own css with ```frame_set_css(...)```, each input has its own classname defined as follows:
