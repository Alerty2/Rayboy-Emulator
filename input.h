#ifndef INPUT_H
#define INPUT_H

#include "flags.h"

extern bool button_a;
extern bool button_b;
extern bool button_start;
extern bool button_select;
extern bool dpad_up;
extern bool dpad_down;
extern bool dpad_left;
extern bool dpad_right;

// Changes I/O memory depending on the keys that are pressed
void update_input(uint8_t wRAM[]);

#endif
