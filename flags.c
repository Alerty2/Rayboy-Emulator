#include "flags.h"
#include "instructions.h"  // ← para FLAG_C, FLAG_H, etc.

void set_flag(uint8_t* flags, uint8_t flag){
    *flags |= flag;
}

void unset_flag(uint8_t* flags, uint8_t flag){
    *flags &= ~flag;
}

void reset_flags(uint8_t* flags){
    *flags &= ~(FLAG_C | FLAG_H | FLAG_N | FLAG_Z);
}
