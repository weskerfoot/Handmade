## What is this?

My attempt at following along with [https://handmadehero.org](https://handmadehero.org) but instead of win32, it is built on X and Cairo, and uses UNIX APIs.

* Basic double buffered rendering with Cairo works. Cairo handles rendering surfaces to the window created by XCB, and this code handles writing pixels / handling X events, and swapping buffers.
* Basic keyboard input works using XCB.
* Basic audio using the SDL2 audio backend works (runs in a separate thread).

## Dependencies
* xcb/xlib
* libcairo
* sdl2
* sdl2_mixer
