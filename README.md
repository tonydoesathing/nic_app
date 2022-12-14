# nic_app
Supplies application layer support for our custom network in CP341

## Introduction
This C project establishes the application layer for the custom CP341 network.
- `nic_app.c` is the library file that should be imported to provide sending and receiving functions for an app.
- `nic_app.h` is the header file for the above module.

## Building and Running
Keeping with the standards of the class, this code should be built and run on a Raspberry Pi 3B+ with [pigpio](https://abyz.me.uk/rpi/pigpio/index.html) installed. 
To build the CLI, run `gcc -pthread -o chat chat.c nic_app.c -lpigpiod_if2` in the `src` directory.
To run the CLI, then call `./chat`.

Note: in order to make network calls, the router from [nic_net](https://github.com/Jessicat-H/nic_net) must be running on the computer you wish to run the application on. Follow the instructions in that repository for message organization.

## Authors
The following code was written by [Jessica Hannebert](https://github.com/Jessicat-H), [Dylan Chapell](https://github.com/dylanchapell), and [Tony Mastromarino](https://github.com/tonydoesathing).
