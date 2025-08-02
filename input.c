#include "input.h"
void update_input(void) {
    uint8_t joyp_val = read_byte(0xFF00);

    // Get selection bits (P14/P15) from memory
    uint8_t selection = joyp_val & 0x30;  // Keep bits 4–5

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

    joyp_val = input;
    write_byte(0xFF00, input);
}
