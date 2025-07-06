#include "input.h"
#define FLAG_C 0
#define FLAG_H 1
#define FLAG_N 3
#define FLAG_Z 4
// CPU
typedef union {
    struct {
        uint8_t F; // flags (lower byte)
        uint8_t A; // accumulator (upper byte)
    };
    uint16_t AF;
} reg_af_t;

typedef union {
    struct {
        uint8_t C;
        uint8_t B;
    };
    uint16_t BC;
} reg_bc_t;

typedef union {
    struct {
        uint8_t E;
        uint8_t D;
    };
    uint16_t DE;
} reg_de_t;

typedef union {
    struct {
        uint8_t L;
        uint8_t H;
    };
    uint16_t HL;
} reg_hl_t;

typedef struct {
    reg_af_t af;
    reg_bc_t bc;
    reg_de_t de;
    reg_hl_t hl;
    uint16_t pc;
    uint16_t sp;
    int cycles;
} CPU;

// Loads (LD). Instructions for copying a value into other place
void load_r16_n16(uint16_t* reg, uint8_t memory[], CPU* cpu);
void load_p16_n8(uint16_t* reg, uint8_t memory[], CPU* cpu);
void load_p16_r8(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu);
void load_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu);
void load_a16_SP(uint8_t memory[], CPU* cpu);
void load_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu);
void load_p16_r8_plus(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu);
void load_p16_r8_minus(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu);
void load_r8_p16_plus(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu);
void load_r8_p16_minus(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu);

// Arithmetic instructions (INC, DEC and ADD). Instructions for doing basic arithmetic operations
void inc_r16(uint16_t* reg, uint8_t memory[], CPU* cpu);
void inc_r8(uint8_t* reg, uint8_t memory[], CPU* cpu);
void inc_p16(uint16_t* reg, uint8_t memory[], CPU* cpu);
void dec_r8(uint8_t* reg, uint8_t memory[], CPU* cpu);
void dec_r16(uint16_t* reg, uint8_t memory[], CPU* cpu);
void dec_p16(uint16_t* reg, uint8_t memory[], CPU* cpu);
void add_r16_r16(uint16_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu);

// Binary operations
void rlca(uint8_t memory[], CPU* cpu);
void rla(uint8_t memory[], CPU* cpu);
void rrca(uint8_t memory[], CPU* cpu);
void rra(uint8_t memory[], CPU* cpu);
void cpl(uint8_t memory[], CPU* cpu);

// Jumps and conditionals
void jump_register_e8(uint8_t memory[], CPU* cpu);
void jump_register_nz_e8(uint8_t memory[], CPU* cpu);
void jump_register_z_e8(uint8_t memory[], CPU* cpu);
void jump_register_nc_e8(uint8_t memory[], CPU* cpu);
void jump_register_c_e8(uint8_t memory[], CPU* cpu);

// Miscelaneous
void dda(uint8_t memory[], CPU* cpu);
