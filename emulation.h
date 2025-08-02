#ifndef EMULATION_H
#define EMULATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "flags.h"
#include "mmu.h"          // para read_byte/write_byte
#include "instructions.h" // para tipo CPU, flags, etc.

int emulate_cycle(CPU* cpu);
void handle_interrupts(CPU* cpu);

#endif
