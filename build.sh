#! /usr/bin/env bash
$CC -Wall --pedantic --std=gnu11 $(pkg-config --cflags --libs sdl2 cairo x11 x11-xcb xcb) "$1"
