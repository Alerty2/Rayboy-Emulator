#include "instructions.h"

void debug_vram_write(uint16_t addr, uint8_t value) {
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        printf("[WRITE VRAM] @%04X = %02X\n", addr, value);
    }
    if (addr >= 0x9800 && addr <= 0x9FFF) {
        printf("[WRITE TILEMAP] @%04X = %02X\n", addr, value);
    }
}

// Loads (LD). Instructions for copying a value into other place
void load_r16_n16(uint16_t* reg, uint8_t memory[], CPU* cpu){ // Copies a 16-bit number into a 16 bits register
    uint8_t low  = memory[cpu->pc++]; // BC
    uint8_t high = memory[cpu->pc++]; // d16
    *reg = (high << 8) | low;
    cpu->cycles += 12;
}
void load_p16_n8(uint16_t* reg, uint8_t memory[], CPU* cpu){ // LD [HL],r8. Copy the value in register r8 into the byte pointed to by HL.
    memory[*reg] += memory[cpu->pc++];
    debug_vram_write(*reg, memory[*reg]);
    printf("[LD p16, n8] WRITE @%04X = %02X\n", *reg, memory[*reg]);
    cpu->cycles += 12;
}
void load_p16_r8(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    memory[*reg] += *reg2;
    debug_vram_write(*reg, *reg2);
    printf("[LD p16, r8] WRITE @%04X = %02X\n", *reg, memory[*reg]);
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
    printf("[LD [a16], SP] WRITE @%04X = %02X (low), @%04X = %02X (high)\n", addr, memory[addr], addr + 1, memory[addr + 1]);
    cpu->cycles += 20;
}
void load_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){ // LD r8,n8. Copy the value n8 into register r8.
    *reg = memory[*reg2];
    cpu->cycles += 8;
}
void load_p16_r8_plus(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){ // LD [HL],r8. Copy the value in register r8 into the byte pointed to by HL.
    memory[*reg] = *reg2;
    debug_vram_write(*reg, *reg2);
    printf("[LD (HL+), r8] WRITE @%04X = %02X\n", *reg, *reg2);
    reg++;
    cpu->cycles += 8;
}
void load_p16_r8_minus(uint16_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    memory[*reg] = *reg2;
    debug_vram_write(*reg, *reg2);
    printf("[LD (HL-), r8] WRITE @%04X = %02X\n", *reg, *reg2);
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
void load_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    *reg = *reg2;
    cpu->cycles += 4;
}
void load_a8_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t addr = memory[cpu->pc++];
    memory[0xFF00 + addr] = *reg;
    debug_vram_write(0xFF00 + addr, *reg);
    printf("[LD [$FF00+%02X], r8] WRITE @%04X = %02X\n", addr, 0xFF00 + addr, *reg);
    cpu->cycles += 12;
}
void load_r8_a8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t addr = memory[cpu->pc++];
    *reg = memory[0xFF00 + addr];
    cpu->cycles += 12;
}
void load_c_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t addr = cpu->bc.C;
    memory[0xFF00 + addr] = *reg;
    debug_vram_write(0xFF00 + addr, *reg);
    printf("[LD [$FF00+C], r8] WRITE @%04X = %02X\n", 0xFF00 + addr, *reg);
    cpu->cycles += 8;
}
void load_r8_c(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t addr = cpu->bc.C;
    *reg = memory[0xFF00 + addr];
    cpu->cycles += 8;
}
void load_a16_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;
    memory[addr] = *reg;
    debug_vram_write(addr, *reg);
    printf("[LD [a16], r8] WRITE @%04X = %02X\n", addr, *reg);

}
void load_r8_a16(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;
    *reg = memory[addr];
}
void load_hl_sp_e8(uint8_t memory[], CPU* cpu){
    int8_t value = (int8_t)memory[cpu->pc++]; // Signed 8-bit offset
    uint16_t sp = cpu->sp;
    uint16_t result = sp + value;

    // Clear Z and N flags
    unset_flag(&cpu->af.F, FLAG_Z);
    unset_flag(&cpu->af.F, FLAG_N);

    // Set H flag if lower nibble carry from bit 3 to bit 4
    if (((sp & 0x0F) + (value & 0x0F)) > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Set C flag if carry from bit 7 to bit 8
    if (((sp & 0xFF) + (value & 0xFF)) > 0xFF)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->hl.HL = result;
    cpu->cycles += 12;
}
void load_sp_hl(uint8_t memory[], CPU* cpu){
    cpu->sp += cpu->hl.HL;
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
    printf("ADD HL, SP executed: HL=0x%04X, SP=0x%04X, Flags: %02X\n", *reg, *reg2, cpu->af.F);
}
void add_sp_e8(uint8_t memory[], CPU* cpu){
    int8_t value = (int8_t)memory[cpu->pc++];  // e8 is signed
    uint16_t sp = cpu->sp;
    uint16_t result = sp + value;

    // Reset Z and N flags
    unset_flag(&cpu->af.F, FLAG_Z);
    unset_flag(&cpu->af.F, FLAG_N);

    // Set H if lower nibble carries from bit 3 to 4
    if (((sp & 0x0F) + (value & 0x0F)) > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Set C if lower byte carries from bit 7 to 8
    if (((sp & 0xFF) + (value & 0xFF)) > 0xFF)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->sp = result;
    cpu->cycles += 16;
}

void add_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu) {
    uint16_t result = *reg + *reg2;

    // Zero flag
    if ((uint8_t)result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag = 0 (because it's an addition)
    unset_flag(&cpu->af.F, FLAG_N);

    // Half-carry: if lower nibble overflows
    if (((*reg & 0x0F) + (*reg2 & 0x0F)) > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if result exceeds 0xFF
    if (result > 0xFF)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = (uint8_t)result;
    cpu->cycles += 4;
}
void add_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[cpu->pc++];
    uint16_t result = *reg + value;

    // Zero flag
    if ((uint8_t)result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag = 0 (because it's an addition)
    unset_flag(&cpu->af.F, FLAG_N);

    // Half-carry: if lower nibble overflows
    if (((*reg & 0x0F) + (value & 0x0F)) > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if result exceeds 0xFF
    if (result > 0xFF)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = (uint8_t)result;
    cpu->cycles += 8;
}
void add_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu) {
    uint16_t result = *reg + memory[*reg2];

    // Zero flag
    if ((uint8_t)result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag = 0 (because it's an addition)
    unset_flag(&cpu->af.F, FLAG_N);

    // Half-carry: if lower nibble overflows
    if (((*reg & 0x0F) + (*reg2 & 0x0F)) > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if result exceeds 0xFF
    if (result > 0xFF)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = (uint8_t)result;
    cpu->cycles += 4;
}
void adc_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu) {
    uint8_t carry = (cpu->af.F & FLAG_C) ? 1 : 0;
    uint16_t result = *reg + *reg2 + carry;

    // Zero flag
    if ((uint8_t)result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag = 0 (because it's an addition)
    unset_flag(&cpu->af.F, FLAG_N);

    // Half-carry: bit 3 carry to bit 4
    if (((*reg & 0x0F) + (*reg2 & 0x0F) + carry) > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: result > 0xFF
    if (result > 0xFF)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = (uint8_t)result;
    cpu->cycles += 4;
}

void adc_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu) {
    uint8_t value = memory[cpu->pc++];
    uint8_t carry = (cpu->af.F & FLAG_C) ? 1 : 0;
    uint16_t result = *reg + value + carry;

    // Zero flag
    if ((uint8_t)result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag = 0 (because it's an addition)
    unset_flag(&cpu->af.F, FLAG_N);

    // Half-carry: bit 3 carry to bit 4
    if (((*reg & 0x0F) + (value & 0x0F) + carry) > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: result > 0xFF
    if (result > 0xFF)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = (uint8_t)result;
    cpu->cycles += 8;
}

void adc_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu) {
    uint8_t carry = (cpu->af.F & FLAG_C) ? 1 : 0;
    uint16_t result = *reg + memory[*reg2] + carry;

    // Zero flag
    if ((uint8_t)result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag = 0 (because it's an addition)
    unset_flag(&cpu->af.F, FLAG_N);

    // Half-carry: bit 3 carry to bit 4
    if (((*reg & 0x0F) + (*reg2 & 0x0F) + carry) > 0x0F)
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: result > 0xFF
    if (result > 0xFF)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = (uint8_t)result;
    cpu->cycles += 4;
}

void sbc_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu) {
    uint8_t carry = (cpu->af.F & FLAG_C) ? 1 : 0;
    uint8_t value = *reg2;
    uint8_t before = *reg;
    uint8_t result = before - value - carry;

    // Zero flag
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag (always set for SBC)
    set_flag(&cpu->af.F, FLAG_N);

    // Half-carry: if borrow from bit 4
    if ((before & 0x0F) < ((value & 0x0F) + carry))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if borrow from bit 8
    if (before < (value + carry))
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = result;
    cpu->cycles += 4;
}
void sbc_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu) {
    uint8_t carry = (cpu->af.F & FLAG_C) ? 1 : 0;
    uint8_t value = memory[cpu->pc++];
    uint8_t before = *reg;
    uint8_t result = before - value - carry;

    // Zero flag
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag (always set for SBC)
    set_flag(&cpu->af.F, FLAG_N);

    // Half-carry: if borrow from bit 4
    if ((before & 0x0F) < ((value & 0x0F) + carry))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if borrow from bit 8
    if (before < (value + carry))
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = result;
    cpu->cycles += 8;
}
void sbc_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu) {
    uint8_t carry = (cpu->af.F & FLAG_C) ? 1 : 0;
    uint8_t value = memory[*reg2];
    uint8_t before = *reg;
    uint8_t result = before - value - carry;

    // Zero flag
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag (always set for SBC)
    set_flag(&cpu->af.F, FLAG_N);

    // Half-carry: if borrow from bit 4
    if ((before & 0x0F) < ((value & 0x0F) + carry))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if borrow from bit 8
    if (before < (value + carry))
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = result;
    cpu->cycles += 4;
}
void sub_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu) {
    uint8_t value = *reg2;
    uint8_t before = *reg;
    uint8_t result = before - value;


    // Zero flag
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag = 1
    set_flag(&cpu->af.F, FLAG_N);

    // Half-carry
    if ((before & 0x0F) < (value & 0x0F))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // If result is negative value
    printf("Before: %d\n", before);
    printf("Value: %d\n", value);
    if (before < value){
        set_flag(&cpu->af.F, FLAG_C);
        printf("FLAG_C ACTIVATED\n");
    }
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = result;
    cpu->cycles += 4;
}
void sub_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu) {
    uint8_t value = memory[cpu->pc++];
    uint8_t before = *reg;
    uint8_t result = *reg - value;

    // Zero flag
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag = 1 (porque es resta)
    set_flag(&cpu->af.F, FLAG_N);

    // Half-carry: si hubo préstamo del bit 4 (nibble bajo)
    if ((before & 0x0F) < (value & 0x0F))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: si el resultado fue negativo
    if (before < value)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = result;
    cpu->cycles += 8;
}


void sub_r8_p16(uint8_t* reg, uint16_t* addr, uint8_t memory[], CPU* cpu) {
    uint8_t value = memory[*addr];
    uint8_t before = *reg;
    uint8_t result = before - value;

    // Zero flag
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    // Subtract flag
    set_flag(&cpu->af.F, FLAG_N);

    // Half-carry
    if ((before & 0x0F) < (value & 0x0F))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry
    if (before < value)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    *reg = result;
    cpu->cycles += 8; // Esto es típico para SUB A, (HL)
}
void cp_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    uint8_t result = *reg - *reg2;
    uint8_t value = *reg2;
    uint8_t before = *reg;

    // Flag Z
    if (result == 0){
        set_flag(&cpu->af.F, FLAG_Z);
    }else{
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    set_flag(&cpu->af.F, FLAG_N);
    // Half-carry: if borrow from bit 4
    if ((before & 0x0F) < (value & 0x0F))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if borrow from bit 8
    if (before < value)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);
}
void cp_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[cpu->pc++];
    uint8_t result = *reg - value;
    uint8_t before = *reg;

    // Flag Z
    if (result == 0){
        set_flag(&cpu->af.F, FLAG_Z);
    }else{
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    set_flag(&cpu->af.F, FLAG_N);
    // Half-carry: if borrow from bit 4
    if ((before & 0x0F) < (value & 0x0F))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if borrow from bit 8
    if (before < value)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);
}
void cp_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){
    uint8_t result = *reg - memory[*reg2];
    uint8_t value = memory[*reg2];
    uint8_t before = *reg;

    // Flag Z
    if (result == 0){
        set_flag(&cpu->af.F, FLAG_Z);
    }else{
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    set_flag(&cpu->af.F, FLAG_N);
    // Half-carry: if borrow from bit 4
    if ((before & 0x0F) < (value & 0x0F))
        set_flag(&cpu->af.F, FLAG_H);
    else
        unset_flag(&cpu->af.F, FLAG_H);

    // Carry: if borrow from bit 8
    if (before < value)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);
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
void rlc_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = *reg;
    uint8_t carry = (value & 0x80) >> 7; // Save bit 7

    value = (value << 1) | carry; // Rotate left, bit 7 as the new bit 0

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 8;
}
void rl_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = *reg;
    uint8_t carry = (value & 0x80) >> 7; // Save bit 7 for calculate CARRY_FLAG
    uint8_t carry_flag = (cpu->af.F & FLAG_C) ? 1 : 0;
    value = (value << 1) | carry_flag; // Rotate left, bit 7 as the new bit 0

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 8;
}
void rl_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[*reg];
    uint8_t carry = (value & 0x80) >> 7; // Save bit 7 for calculate CARRY_FLAG
    uint8_t carry_flag = (cpu->af.F & FLAG_C) ? 1 : 0;
    value = (value << 1) | carry_flag; // Rotate left, bit 7 as the new bit 0

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 16;
}
void rlc_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[*reg];
    uint8_t carry = (value & 0x80) >> 7; // Save bit 7

    value = (value << 1) | carry; // Rotate left, bit 7 as the new bit 0

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 16;
}
void rrc_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = *reg;
    uint8_t carry = value & 0x01; // Save bit 7

    value = (value >> 1) | (carry << 7); // Rotate right, bit 0 as the new bit 7

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 8;
}
void rr_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = *reg;
    uint8_t carry = value & 0x01; // Save bit 0
    uint8_t carry_flag = (cpu->af.F & FLAG_C) ? 1 : 0;

    value = (value >> 1) | (carry_flag << 7); // Rotate right, carry flag as the new bit 7.

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 8;
}
void rr_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[*reg];
    uint8_t carry = value & 0x01; // Save bit 0
    uint8_t carry_flag = (cpu->af.F & FLAG_C) ? 1 : 0;

    value = (value >> 1) | (carry_flag << 7); // Rotate right, carry flag as the new bit 7.

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 16;
}
void rrc_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[*reg];
    uint8_t carry = value & 0x01; // Save bit 7

    value = (value >> 1) | (carry << 7); // Rotate right, bit 0 as the new bit 7

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 16;
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
void and_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    uint8_t result = *reg & *reg2;
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always set in AND
    set_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 4;
}
void and_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[cpu->pc++];
    uint8_t result = *reg & value;
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always set in AND
    set_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 8;
}
void and_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){
    uint8_t result = *reg & memory[*reg2];
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always set in AND
    set_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 4;
}
void xor_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    uint8_t result = *reg ^ *reg2;
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always unset in XOR
    unset_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 4;
}
void xor_r8_n8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[cpu->pc++];
    uint8_t result = *reg ^ value;
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always set in AND
    set_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 8;
}
void xor_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){
    uint8_t result = *reg ^ memory[*reg2];
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always unset in XOR
    unset_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 4;
}
void or_r8_r8(uint8_t* reg, uint8_t* reg2, uint8_t memory[], CPU* cpu){
    uint8_t result = *reg | *reg2;
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always unset in OR
    unset_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 4;
}
void or_r8_p16(uint8_t* reg, uint16_t* reg2, uint8_t memory[], CPU* cpu){
    uint8_t result = *reg | memory[*reg2];
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always unset in OR
    unset_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 4;
}
void or_r8_n8(uint8_t* reg,  uint8_t memory[], CPU* cpu){
    uint8_t value = memory[cpu->pc++];
    uint8_t result = *reg | value;
    *reg = result;
    // Z flag: set if result is zero
    if (result == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);
    // N and C are always reset
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_C);

    // H is always set in AND
    set_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 8;
}
void sla_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = *reg;
    uint8_t carry = (value & 0x80) >> 7; // Save bit 7
    value = (value << 1); // Rotate left, bit 0 = 0

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 8;
}
void sla_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[*reg];
    uint8_t carry = (value & 0x80) >> 7; // Save bit 7
    value = (value << 1); // Rotate left, bit 0 = 0

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 16;
}
void sra_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = *reg;
    uint8_t carry = value & 0x01; // Save bit 0
    uint8_t msb = value & 0x80; // Save bit 7
    value = (value >> 1) | msb; // Rotate right.

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 8;
}
void sra_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[*reg];
    uint8_t carry = value & 0x01; // Save bit 0
    uint8_t msb = value & 0x80; // Save bit 7
    value = (value >> 1) | msb; // Rotate right.

    *reg = value;

    // Flags
    if (value == 0)
        set_flag(&cpu->af.F, FLAG_Z);
    else
        unset_flag(&cpu->af.F, FLAG_Z);

    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);

    if (carry)
        set_flag(&cpu->af.F, FLAG_C);
    else
        unset_flag(&cpu->af.F, FLAG_C);

    cpu->cycles += 16;
}
void swap_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = *reg;
    uint8_t result = (value >> 4) | (value << 4);
    *reg = result;

    if (result == 0){
        set_flag(&cpu->af.F, FLAG_Z);
    }else{
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);
    unset_flag(&cpu->af.F, FLAG_C);
    cpu->cycles += 8;
}
void swap_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[*reg];
    uint8_t result = (value >> 4) | (value << 4);
    memory[*reg] = result;

    if (result == 0){
        set_flag(&cpu->af.F, FLAG_Z);
    }else{
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);
    unset_flag(&cpu->af.F, FLAG_C);
    cpu->cycles += 16;
}
void srl_r8(uint8_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = *reg;
    uint8_t carry = value & 0x01; // Save bit 0
    uint8_t result = (value >> 1);
    *reg = result;
    // Flags
    if (result == 0){
        set_flag(&cpu->af.F, FLAG_Z);
    }else{
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);
    if (carry){
        set_flag(&cpu->af.F, FLAG_C);
    }else{
        unset_flag(&cpu->af.F, FLAG_C);
    }
    cpu->cycles += 8;
}
void srl_p16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t value = memory[*reg];
    uint8_t carry = value & 0x01; // Save bit 0
    uint8_t result = (value >> 1);
    memory[*reg] = result;
    // Flags
    if (result == 0){
        set_flag(&cpu->af.F, FLAG_Z);
    }else{
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    unset_flag(&cpu->af.F, FLAG_N);
    unset_flag(&cpu->af.F, FLAG_H);
    if (carry){
        set_flag(&cpu->af.F, FLAG_C);
    }else{
        unset_flag(&cpu->af.F, FLAG_C);
    }
    cpu->cycles += 16;
}
void bit_u3_r8(uint8_t bit, uint8_t* reg, uint8_t memory[], CPU* cpu){
    // If bit number bit is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
    if (!(*reg & (1 << bit))) {
        set_flag(&cpu->af.F, FLAG_Z);
    } else {
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    unset_flag(&cpu->af.F, FLAG_N);
    set_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 8;
}
void bit_u3_p16(uint8_t bit, uint16_t* reg, uint8_t memory[], CPU* cpu){
    // If bit number bit is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
    if (!(memory[*reg] & (1 << bit))) {
        set_flag(&cpu->af.F, FLAG_Z);
    } else {
        unset_flag(&cpu->af.F, FLAG_Z);
    }
    unset_flag(&cpu->af.F, FLAG_N);
    set_flag(&cpu->af.F, FLAG_H);
    cpu->cycles += 8;
}
void res_u3_r8(uint8_t bit, uint8_t* reg, uint8_t memory[], CPU* cpu) {
    *reg &= ~(1 << bit);
    cpu->cycles += 8;
}
void res_u3_p16(uint8_t bit, uint16_t* reg, uint8_t memory[], CPU* cpu) {
    memory[*reg] &= ~(1 << bit);
    cpu->cycles += 16;
}
void set_u3_r8(uint8_t bit, uint8_t* reg, uint8_t memory[], CPU* cpu) {
    *reg |= (1 << bit);
    cpu->cycles += 8;
}
void set_u3_p16(uint8_t bit, uint16_t* reg, uint8_t memory[], CPU* cpu) {
    memory[*reg] |= (1 << bit);
    cpu->cycles += 16;
}


// Jump, confitionals, etc...
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
    int8_t offset = (int8_t)memory[cpu->pc + 1];

    if (!(cpu->af.F & FLAG_Z)) {
        cpu->pc += 2 + offset;
        cpu->cycles += 12;
    } else {
        cpu->pc += 2;
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
    int8_t offset = (int8_t)memory[cpu->pc++];
    if (!(cpu->af.F & FLAG_C)) {
        cpu->pc += offset;
        cpu->cycles += 12;
    } else {
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
void return_nz(uint8_t memory[], CPU* cpu){ // Goes to adress saved in sp (stack)
    // If FLAG ZERO = 0
    if (!(cpu->af.F & FLAG_Z)) {
        uint8_t low = memory[cpu->sp];
        uint8_t high = memory[cpu->sp + 1];
        cpu->pc = (high << 8) | low;
        cpu->sp += 2;
        cpu->cycles += 20;
    } else {
        cpu->cycles += 8;
    }
}
void return_nc(uint8_t memory[], CPU* cpu){ // Goes to adress saved in sp (stack)
    // If FLAG CARRY = 0
    if (!(cpu->af.F & FLAG_C)) {
        uint8_t low = memory[cpu->sp];
        uint8_t high = memory[cpu->sp + 1];
        cpu->pc = (high << 8) | low;
        cpu->sp += 2;
        cpu->cycles += 20;
    } else {
        cpu->cycles += 8;
    }
}
void return_z(uint8_t memory[], CPU* cpu){ // Goes to adress saved in sp (stack)
    // If FLAG ZERO = 1
    if (cpu->af.F & FLAG_Z) {
        uint8_t low = memory[cpu->sp];
        uint8_t high = memory[cpu->sp + 1];
        cpu->pc = (high << 8) | low;
        cpu->sp += 2;
        cpu->cycles += 20;
    } else {
        cpu->cycles += 8;
    }
}
void return_c(uint8_t memory[], CPU* cpu){ // Goes to adress saved in sp (stack)
    // If FLAG ZERO = 1
    if (cpu->af.F & FLAG_C) {
        uint8_t low = memory[cpu->sp];
        uint8_t high = memory[cpu->sp + 1];
        cpu->pc = (high << 8) | low;
        cpu->sp += 2;
        cpu->cycles += 20;
    } else {
        cpu->cycles += 8;
    }
}
void return_(uint8_t memory[], CPU* cpu){ // Goes to adress saved in sp (stack)

    uint8_t low = memory[cpu->sp];
    uint8_t high = memory[cpu->sp + 1];
    cpu->pc = (high << 8) | low;
    cpu->sp += 2;
    cpu->cycles += 16;
}
void pop_r16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    // Load in reg the value pointed in sp
    uint8_t low = memory[cpu->sp++];
    uint8_t high = memory[cpu->sp++];
    *reg = (high << 8) | low;
    cpu->cycles += 12;
}
void jump_pointer_nz_a16(uint8_t memory[], CPU* cpu){
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;
    if (!(cpu->af.F & FLAG_Z)) {
        cpu->pc = (high << 8) | low;
        cpu->cycles += 16;
    } else {
        cpu->cycles += 12;
    }
}
void jump_pointer_nc_a16(uint8_t memory[], CPU* cpu){
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;
    if (!(cpu->af.F & FLAG_C)) {
        cpu->pc = (high << 8) | low;
        cpu->cycles += 16;
    } else {
        cpu->cycles += 12;
    }
}
void jump_pointer_z_a16(uint8_t memory[], CPU* cpu){
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;
    if (cpu->af.F & FLAG_Z) {
        cpu->pc = (high << 8) | low;
        cpu->cycles += 16;
    } else {
        cpu->cycles += 12;
    }
}
void jump_pointer_c_a16(uint8_t memory[], CPU* cpu){
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;
    if (cpu->af.F & FLAG_C) {
        cpu->pc = (high << 8) | low;
        cpu->cycles += 16;
    } else {
        cpu->cycles += 12;
    }
}
void jump_pointer_a16(uint8_t memory[], CPU* cpu){
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    cpu->pc = (high << 8) | low;
    cpu->cycles += 16;
}
void jump_pointer_HL(uint8_t memory[], CPU* cpu){
    cpu->pc = cpu->hl.HL;
    cpu->cycles += 4;
}
void call_nz_a16(uint8_t memory[], CPU* cpu) {
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;

    if (!(cpu->af.F & FLAG_Z)) {
        cpu->sp--; // push high byte of PC
        memory[cpu->sp] = (cpu->pc >> 8) & 0xFF;
        cpu->sp--; // push low byte of PC
        memory[cpu->sp] = cpu->pc & 0xFF;

        cpu->pc = addr;
        cpu->cycles += 24;
    } else {
        cpu->cycles += 12;
    }
}
void call_nc_a16(uint8_t memory[], CPU* cpu) {
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;

    if (!(cpu->af.F & FLAG_C)) {
        cpu->sp--; // push high byte of PC
        memory[cpu->sp] = (cpu->pc >> 8) & 0xFF;
        cpu->sp--; // push low byte of PC
        memory[cpu->sp] = cpu->pc & 0xFF;

        cpu->pc = addr;
        cpu->cycles += 24;
    } else {
        cpu->cycles += 12;
    }
}
void call_z_a16(uint8_t memory[], CPU* cpu) {
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;

    if (cpu->af.F & FLAG_Z) {
        cpu->sp--; // push high byte of PC
        memory[cpu->sp] = (cpu->pc >> 8) & 0xFF;
        cpu->sp--; // push low byte of PC
        memory[cpu->sp] = cpu->pc & 0xFF;

        cpu->pc = addr;
        cpu->cycles += 24;
    } else {
        cpu->cycles += 12;
    }
}
void call_c_a16(uint8_t memory[], CPU* cpu) {
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;

    if (cpu->af.F & FLAG_C) {
        cpu->sp--; // push high byte of PC
        memory[cpu->sp] = (cpu->pc >> 8) & 0xFF;
        cpu->sp--; // push low byte of PC
        memory[cpu->sp] = cpu->pc & 0xFF;

        cpu->pc = addr;
        cpu->cycles += 24;
    } else {
        cpu->cycles += 12;
    }
}
void call_a16(uint8_t memory[], CPU* cpu){
    uint8_t low = memory[cpu->pc++];
    uint8_t high = memory[cpu->pc++];
    uint16_t addr = (high << 8) | low;
    cpu->sp--; // push high byte of PC
    memory[cpu->sp] = (cpu->pc >> 8) & 0xFF;
    cpu->sp--; // push low byte of PC
    memory[cpu->sp] = cpu->pc & 0xFF;

    cpu->pc = addr;
    cpu->cycles += 24;
}
void push_r16(uint16_t* reg, uint8_t memory[], CPU* cpu){
    uint8_t high = (*reg >> 8) & 0xFF;
    uint8_t low  = *reg & 0xFF;
    cpu->sp--; // push high byte of PC
    memory[cpu->sp] = high;
    cpu->sp--; // push low byte of PC
    memory[cpu->sp] = low;
    cpu->cycles += 16;
}
void rst_vec(uint8_t vec, uint8_t memory[], CPU* cpu){
    // Push current PC onto the stack (low byte first, then high byte)
    cpu->sp--;
    memory[cpu->sp] = (cpu->pc >> 8) & 0xFF; // high byte
    cpu->sp--;
    memory[cpu->sp] = cpu->pc & 0xFF;        // low byte

    cpu->pc = vec; // Jump to fixed vector (0x00, 0x08, ..., 0x38)
    cpu->cycles += 16;
}
// Miscelanous
void dda(uint8_t memory[], CPU* cpu){
    // TODO: Complicated instruction
}
