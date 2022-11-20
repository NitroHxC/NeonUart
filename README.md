# NeonUart

## What is it?
This is my take at developing a *(yet another)* plug-n-play and easy to understand UART protocol.

Usually I find myself in need of a fast implementation just to send small commands between Microcontrollers, and having a simple "drag-n-drop-n-play" library with few API functions for TX and RX some data is of paramount importance. 

I wanted something in plain C as close as an "header-only" lib (not really, just .h and .c) with no dependencies whatsoever.

Even if I work with MCUs, I developed and tested using the MSVC console app you find in this repo... 'cause it's just faster!

## How does it work?

The library is as easy as defining your messages and functions, instantiate an object and link some callbacks.

#### Frame structure

Uses 2 start/header/magic bytes , 2 bytes for defining a type (the family byte is hardcoded to 0x80 for now, *stay tuned*), 2 bytes of length and 2 bytes of simple checksum

``` 
 0xEC 0x9D <family> <type> <lenL> <lenH> <...payload...> <crc1><crc2> 
 ```
#### Message definitions

So basically we just define few things that are going to be linked together by calling the `neon_define_message(...)` function:
- the msg struct (the actual stuff you want to send), with configurable max size
- the `msg ID` (between 0 and 255 . A "Family" byte is already there for future dev, so we'll have up to 65535 messages)
- the msg struct `sizeof()` ... can't avoid this, see below for *the Macro trick*
- an application callback function that handles the stuff when received

To ease the usage a bit, I've added 2 macros for defining the Struct and the ID at the same time:
- `NEON_DEF(NAME,TYPE)` in one line it defines the struct name with a _t suffix and also an enum containing the TYPE/ID
- `NEON_MSG(NAME)` extracts the `sizeof(), TYPE` to pass it in the arg/param list of functions

Example usage for message definition:
```
typedef struct {
    uint16_t field1;
    uint16_t field2;
} NEON_DEF(myMessageName, 0x01);
```

Example usage linking msg to callback:

```
neon_define_message(&parser_object, NEON_MSG(myMessageName), callback);
```

Then we'll have our regular loop feeding the parser with the `neon_parse_char(...)` function.

When a msg is parsed, its payload is dispatched to an handler function that can pack/unpack the message based on the sizeof() that we linked to the specific msg type, and then will call the right application callback.

Refer to `example1.cpp` for a MSVC test implementation.

Refer to `example2.ino` for a minimal Arduino implementation. 



## Things to be aware of

- beware of **packed** or **unpacked** structs, you need to be consistent on your devices to avoid byte alignment problems
- check the different endianness of the devices that are communicating and act accordingly when packing and unpacking data in your application

## Future development

For the very first implementation I just dynamically allocated the full array for the callback registers, but this could be implemented as an hash table or even just a linked list, for minimal memory footprint.

The next step could be to implement a variation of the protocol with start-byte, stop-byte and byte stuffing, and be able to select which type of protocol to use.

Then, for more complex protocols, a message definition in XML or JSON could be parsed to generate the library and remove the "struggle" of having to manually link ID, struct and callback in the application init code.

