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
void update_input(uint8_t wRAM[]) {
    uint8_t *joyp = &wRAM[0xFF00 - 0xC000];

    // Get selection bits (P14/P15) from memory
    uint8_t selection = *joyp & 0x30;  // Keep bits 4–5

    // Start with upper 4 bits preserved, lower 4 bits = all unpressed (1)
    uint8_t input = selection | 0x0F;

    // Buttons selected (P15 = 0)
    if (!(selection & (1 << 5))) {
        if (IsKeyDown(KEY_X))  input &= ~(1 << 3);  // Start
        if (IsKeyDown(KEY_Z))  input &= ~(1 << 2);  // Select
        if (IsKeyDown(KEY_S))  input &= ~(1 << 1);  // B
        if (IsKeyDown(KEY_A))  input &= ~(1 << 0);  // A
    }

    // D-pad selected (P14 = 0)
    if (!(selection & (1 << 4))) {
        if (IsKeyDown(KEY_DOWN)) input &= ~(1 << 3);
        if (IsKeyDown(KEY_UP))   input &= ~(1 << 2);
        if (IsKeyDown(KEY_LEFT)) input &= ~(1 << 1);
        if (IsKeyDown(KEY_RIGHT))input &= ~(1 << 0);
    }

    *joyp = input;
}


#endif
