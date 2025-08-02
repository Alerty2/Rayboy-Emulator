#ifndef FLAGS_H
#define FLAGS_H
#include <stdint.h>
// Activates the selected flag
void set_flag(uint8_t* flags, uint8_t flag);
// Deactivates the selected flag
void unset_flag(uint8_t* flags, uint8_t flag);

// Resets all flags
void reset_flags(uint8_t* flags);
#endif
