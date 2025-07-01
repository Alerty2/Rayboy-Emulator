#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

// Activates the selected flag
void set_flag(uint8_t* flags, int flag){
    switch(flag){
        case 0: // Carry flag
            *flags |= 1 << 4;
            break;
        case 1: // Half Carry flag (BCD)
            *flags |= 1 << 5;
            break;
        case 3: // Subtraction flag (BCD)
            *flags |= 1 << 6;
            break;
        case 4: // Zero flag
            *flags |= 1 << 7;
            break;
        default:
            printf("Incorrect flag: %d", flag);
            break;
    }
}
// Deactivates the selected flag
void unset_flag(uint8_t* flags, uint8_t flag){
    switch(flag){
        case 0: *flags &= ~(1 << 4); break; // Carry flag
        case 1: *flags &= ~(1 << 5); break; // Half-carry
        case 3: *flags &= ~(1 << 6); break; // Subtraction
        case 4: *flags &= ~(1 << 7); break; // Zero
        default:
            printf("Incorrect flag: %d\n", flag);
            break;
    }
}

// Resets all flags
void reset_flags(uint8_t* flags){
    *flags &= ~(1 << 4); // Carry
    *flags &= ~(1 << 5); // Half-carry
    *flags &= ~(1 << 6); // Subtraction
    *flags &= ~(1 << 7); // Zero
}
