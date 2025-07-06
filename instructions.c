#include "instructions.h"

// Loads (LD). Instructions for copying a value into other place
void load_r16_n16(uint16_t* reg, uint8_t memory[], CPU* cpu){ // Copies a 16-bit number into a 16 bits register
    uint8_t low  = memory[cpu->pc++]; // BC
    uint8_t high = memory[cpu->pc++]; // d16
    *reg = (high << 8) | low;
    cpu->cycles += 12;
}
void load_p16_n8(uint16_t* reg, uint8_t memory[], CPU* cpu){ // LD [HL],r8. Copy the value in register r8 into the byte pointed to by HL.
    memory[*reg] += memory[cpu->pc++];
    cpu->cycles += 12;
}
void load_p16_r8(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    memory[*reg] += *reg2;
    cpu->cycles += 8;
}
void load_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu){ // LD r8,n8. Copy the value n8 into register r8.
    *reg = memory[cpu->pc++];
    cpu->cycles += 8;
}
void load_a16_SP(uint8_t memory[], CPU* cpu){  // LD [a16], SP . Load SP into immediate 16-bit value
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;

    memory[addr] = cpu->sp & 0xFF; // Low
    memory[addr + 1] = (cpu->sp >> 8); // High
    cpu->cycles += 20;
}
void load_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){ // LD r8,n8. Copy the value n8 into register r8.
    *reg = memory[*reg2];
    cpu->cycles += 8;
}
void load_p16_r8_plus(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){ // LD [HL],r8. Copy the value in register r8 into the byte pointed to by HL.
    memory[*reg] = *reg2;
    reg++;
    cpu->cycles += 8;
}
void load_p16_r8_minus(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    memory[*reg] = *reg2;
    reg--;
    cpu->cycles += 8;
}
void load_r8_p16_plus(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){ // LD [HL],r8. Copy the value in register r8 into the byte pointed to by HL.
    *reg = memory[*reg2];
    reg2++;
    cpu->cycles += 8;
}
void load_r8_p16_minus(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){
    *reg = memory[*reg2];
    reg--;
    cpu->cycles += 8;
}
// Arithmetic instructions (INC, DEC and ADD). Instructions for doing basic arithmetic operations
void inc_r16(uint16_t* reg, uint8_t memory[], CPU* cpu){ // INC r16. Increments r16 by 1
    *reg+= 1;
    cpu->cycles += 8;
}
void inc_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){ // INC r8. Increments r8 by 1
    uint8_t before = *reg;
    uint8_t result = before + 1;
    *reg = result;

    // Zero flag
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag: cleared on INC
    unset_flag(&cpu->af.F, FLAG_N);

    // Half-carry flag: set if lower nibble overflowed
    if ((before & 0x0F) + 1 > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 4;
}
void inc_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    memory[*reg] += 1;
    cpu->cycles += 12;
}
void dec_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){ // DEC r8. Decrements r8 by 1
    uint8_t before = *reg;
    uint8_t result = before - 1;
    *reg = result;

    // Z flag
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // N flag
    set_flag(&cpu->af.F, FLAG_N);

    // H flag (borrow from bit 4)
    if ((before & 0x0F) == 0x00)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    cpu->cycles += 4;
}
void dec_r16(uint16_t* reg, uint8_t memory[], CPU* cpu){ // DEC r16. Decrements r16 by 1
    *reg -= 1;
    cpu->cycles += 8;
}
void dec_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    memory[*reg] -= 1;
    cpu->cycles += 12;
}
void add_r16_r16(uint16_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){ // ADD r16, r16. Adds r16 to r16
    uint32_t result = *reg + *reg2;

    // Flags
    unset_flag(&cpu->af.F, 6); // N = 0

    // Half Carry: if carry from bit 11
    if ((cpu->hl.HL & 0x0FFF) + (*reg2 & 0x0FFF) > 0x0FFF)
        set_flag(&cpu->af.F, 5); // H
    else
        unset_flag(&cpu->af.F, 5);

    // Carry: if carry from bit 15
    if (result > 0xFFFF)
        set_flag(&cpu->af.F, 4); // C
    else
        unset_flag(&cpu->af.F, 4);

    *reg = result & 0xFFFF;
    cpu->cycles += 8;
}


// Binary operations
void rlca(uint8_t memory[], CPU* cpu){ // RLCA. Rotate register A left.
    uint8_t old_bit7 = (cpu->af.A & 0x80) >> 7; // Save bit 7

    cpu->af.A = (cpu->af.A << 1) | old_bit7;    // Move to the left and insert bit 7 in bit 0.

    // Update flags
    unset_flag(&cpu->af.F, FLAG_Z); // Z = 0
    unset_flag(&cpu->af.F, FLAG_N); // N = 0
    unset_flag(&cpu->af.F, FLAG_H); // H = 0

    if (old_bit7)
        set_flag(&cpu->af.F, 4); // C = 1 if bit 7 = 1
    else
        unset_flag(&cpu->af.F, FLAG_C); // C = 0 si no

    cpu->cycles += 4;
}
void rla(uint8_t memory[], CPU* cpu){ // RLA. Moves to the left and saves bit 7 in carry
    uint8_t old_bit7 = (cpu->af.A & 0x80) >> 7; // Save bit 7
    uint8_t old_carry = (cpu->af.F & (1 << 4)) ? 1 : 0; // Extracts old carry from Carry Flag
    cpu->af.A = (cpu->af.A << 1) | old_carry;    // Move to the left and insert bit 7 in bit 0.

    // Update flags
    unset_flag(&cpu->af.F, FLAG_Z); // Z = 0
    unset_flag(&cpu->af.F, FLAG_N); // N = 0
    unset_flag(&cpu->af.F, FLAG_H); // H = 0

    if (old_bit7)
        set_flag(&cpu->af.F, FLAG_C); // C = 1 if bit 7 = 1
    else
        unset_flag(&cpu->af.F, FLAG_C); // C = 0 if not

    cpu->cycles += 4;
}
void rrca(uint8_t memory[], CPU* cpu){ //  RRCA. Rotate register A right.
    uint8_t old_bit0 = cpu->af.A & 0x01; // Save bit 0
    cpu->af.A = (cpu->af.A >> 1) | (old_bit0 << 7);    // Move to the right and insert bit 0 in bit 7.

    // Update flags
    unset_flag(&cpu->af.F, FLAG_Z); // Z = 0
    unset_flag(&cpu->af.F, FLAG_N); // N = 0
    unset_flag(&cpu->af.F, FLAG_H); // H = 0

    if (old_bit0)
        set_flag(&cpu->af.F, FLAG_C); // C = 1 if bit 0 = 1
    else
        unset_flag(&cpu->af.F, FLAG_C); // C = 0 si no

    cpu->cycles += 4;
}
void rra(uint8_t memory[], CPU* cpu){ // RRA. Rotate A right through carry
    uint8_t old_bit0 = cpu->af.A & 0x01;
    uint8_t carry = (cpu->af.F & FLAG_C) ? 1 : 0;

    cpu->af.A = (cpu->af.A >> 1) | (carry << 7);

    // Flags
    unset_flag(&cpu->af.F, FLAG_Z); // Z = 0
    unset_flag(&cpu->af.F, FLAG_N); // N = 0
    unset_flag(&cpu->af.F, FLAG_H); // H = 0

    if (old_bit0)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 4;
}
void cpl(uint8_t memory[], CPU* cpu){ // CPL. Biwise not
    cpu->af.A = ~cpu->af.A; // Bitwise NOT over A register
    set_flag(&cpu->af.F, FLAG_N);
    set_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 4;
}
void jump_register_e8(uint8_t memory[], CPU* cpu){
    cpu->pc = cpu->pc += (int8_t)memory[cpu->pc++];
    cpu->cycles += 12;
}
void jump_register_nz_e8(uint8_t memory[], CPU* cpu){
    int8_t add_adress = (int8_t)memory[cpu->pc++];
    if (!(cpu->af.F & FLAG_Z)){
        cpu->pc += add_adress;
        cpu->cycles += 12;
    }else{
        cpu->cycles += 8;
    }
}
void jump_register_z_e8(uint8_t memory[], CPU* cpu){
    int8_t add_adress = (int8_t)memory[cpu->pc++];
    if (cpu->af.F & FLAG_Z){
        cpu->pc += add_adress;
        cpu->cycles += 12;
    }else{
        cpu->cycles += 8;
    }
}
void jump_register_nc_e8(uint8_t memory[], CPU* cpu){
    int8_t add_adress = (int8_t)memory[cpu->pc++];
    if (!(cpu->af.F & FLAG_C)){
        cpu->pc += add_adress;
        cpu->cycles += 12;
    }else{
        cpu->cycles += 8;
    }
}
void jump_register_c_e8(uint8_t memory[], CPU* cpu){
    int8_t add_adress = (int8_t)memory[cpu->pc++];
    if (cpu->af.F & FLAG_C){
        cpu->pc += add_adress;
        cpu->cycles += 12;
    }else{
        cpu->cycles += 8;
    }
}
// Miscelanous
void dda(uint8_t memory[], CPU* cpu){
    // TODO: Complicated instruction
}
