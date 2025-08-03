#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "flags.h"

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

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
    bool ime;
    bool halted;
    int cycles;
} CPU;
void debug_vram_write(uint16_t addr, uint8_t value);

// Loads (LD). Instructions for copying a value into other place
void load_r16_n16(uint16_t* reg, CPU* cpu);
void load_p16_n8(uint16_t* reg, CPU* cpu);
void load_p16_r8(uint16_t* reg, uint8_t* reg2, CPU* cpu);
void load_r8_n8(uint8_t* reg, CPU* cpu);
void load_a16_SP(CPU* cpu);
void load_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void load_p16_r8_plus(uint16_t* reg, uint8_t* reg2, CPU* cpu);
void load_p16_r8_minus(uint16_t* reg, uint8_t* reg2, CPU* cpu);
void load_r8_p16_plus(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void load_r8_p16_minus(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void load_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void load_a8_r8(uint8_t* reg, CPU* cpu);
void load_r8_a8(uint8_t* reg, CPU* cpu);
void load_a16_r8(uint8_t* reg, CPU* cpu);
void load_r8_a16(uint8_t* reg, CPU* cpu);
void load_c_r8(uint8_t* reg, CPU* cpu);
void load_r8_c(uint8_t* reg, CPU* cpu);
void load_hl_sp_e8(CPU* cpu);
void load_sp_hl(CPU* cpu);

// Arithmetic instructions (INC, DEC and ADD). Instructions for doing basic arithmetic operations
void inc_r16(uint16_t* reg, CPU* cpu);
void inc_r8(uint8_t* reg, CPU* cpu);
void inc_p16(uint16_t* reg, CPU* cpu);
void dec_r8(uint8_t* reg, CPU* cpu);
void dec_r16(uint16_t* reg, CPU* cpu);
void dec_p16(uint16_t* reg, CPU* cpu);
void add_r16_r16(uint16_t* reg, uint16_t* reg2, CPU* cpu);
void add_sp_e8(CPU* cpu);
void add_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void add_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void add_r8_n8(uint8_t* reg, CPU* cpu);
void adc_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void adc_r8_n8(uint8_t* reg, CPU* cpu);
void adc_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void sub_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void sub_r8_n8(uint8_t* reg, CPU* cpu);
void sub_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void sbc_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void sbc_r8_n8(uint8_t* reg, CPU* cpu);
void sbc_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void cp_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void cp_r8_n8(uint8_t* reg, CPU* cpu);
void cp_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);

// Binary operations
void rlca(CPU* cpu);
void rlc_r8(uint8_t* reg, CPU* cpu);
void rl_r8(uint8_t* reg, CPU* cpu);
void rl_p16(uint16_t* reg, CPU* cpu);
void rlc_p16(uint16_t* reg, CPU* cpu);
void rrc_r8(uint8_t* reg, CPU* cpu);
void rr_r8(uint8_t* reg, CPU* cpu);
void rr_p16(uint16_t* reg, CPU* cpu);
void rrc_p16(uint16_t* reg, CPU* cpu);
void rla(CPU* cpu);
void rrca(CPU* cpu);
void rra(CPU* cpu);
void cpl(CPU* cpu);
void and_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void and_r8_n8(uint8_t* reg, CPU* cpu);
void and_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void xor_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void xor_r8_n8(uint8_t* reg, CPU* cpu);
void xor_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void or_r8_r8(uint8_t* reg, uint8_t* reg2, CPU* cpu);
void or_r8_p16(uint8_t* reg, uint16_t* reg2, CPU* cpu);
void or_r8_n8(uint8_t* reg, CPU* cpu);
void sla_r8(uint8_t* reg, CPU* cpu);
void sla_p16(uint16_t* reg, CPU* cpu);
void sra_r8(uint8_t* reg, CPU* cpu);
void sra_p16(uint16_t* reg, CPU* cpu);
void swap_r8(uint8_t* reg, CPU* cpu);
void swap_p16(uint16_t* reg, CPU* cpu);
void srl_r8(uint8_t* reg, CPU* cpu);
void srl_p16(uint16_t* reg, CPU* cpu);
void bit_u3_r8(uint8_t bit, uint8_t* reg, CPU* cpu);
void bit_u3_p16(uint8_t bit, uint16_t* reg, CPU* cpu);
void res_u3_r8(uint8_t bit, uint8_t* reg, CPU* cpu);
void res_u3_p16(uint8_t bit, uint16_t* reg, CPU* cpu);
void set_u3_r8(uint8_t bit, uint8_t* reg, CPU* cpu);
void set_u3_p16(uint8_t bit, uint16_t* reg, CPU* cpu);

// Jumps and conditionals
void jump_register_e8(CPU* cpu);
void jump_register_nz_e8(CPU* cpu);
void jump_register_z_e8(CPU* cpu);
void jump_register_nc_e8(CPU* cpu);
void jump_register_c_e8(CPU* cpu);
void jump_pointer_nz_a16(CPU* cpu);
void jump_pointer_nc_a16(CPU* cpu);
void jump_pointer_z_a16(CPU* cpu);
void jump_pointer_c_a16(CPU* cpu);
void jump_pointer_a16(CPU* cpu);
void jump_pointer_HL(CPU* cpu);
void call_nz_a16(CPU* cpu);
void call_nc_a16(CPU* cpu);
void call_z_a16(CPU* cpu);
void call_c_a16(CPU* cpu);
void call_a16(CPU* cpu);
void return_nz(CPU* cpu);
void return_nc(CPU* cpu);
void return_z(CPU* cpu);
void return_c(CPU* cpu);
void return_(CPU* cpu);
void pop_r16(uint16_t* reg, CPU* cpu);
void push_r16(uint16_t* reg, CPU* cpu);
void rst_vec(uint8_t vec, CPU* cpu);

// Miscelaneous
void dda(CPU* cpu);
#endif
