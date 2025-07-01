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



void emulate_cycle(uint8_t* memory, CPU* cpu) {
    uint8_t opcode = memory[cpu->pc++];

    switch (opcode) {
        case 0x00: // NOP
            cpu->cycles += 4;
            break;
        case 0x01: { // LD BC, d16. Copies value from BC to d16
            uint8_t low  = memory[cpu->pc++]; // BC
            uint8_t high = memory[cpu->pc++]; // d16
            cpu->bc.BC = (high << 8) | low; // Copies it
            cpu->cycles += 12;
            break;
        }
        case 0x02: { // LD (BC), A. Stores A into the memory at address BC
            memory[cpu->bc.BC] = cpu->af.A;
            cpu->cycles += 8;
            break;
        }
        case 0x03: { // INC BC. Increments BC by 1
            cpu->bc.BC += 1;
            cpu->cycles += 8;
            break;
        }
        case 0x04: { // INC B. Increments B by 1
            memory[cpu->bc.B] += 1;
            // Modifies flags
            if (cpu->bc.B == 0){ // FLAG Z (Carry)
                set_flag(&cpu->af.F, FLAG_Z);
            }else{
                unset_flag(&cpu->af.F, FLAG_Z);
            }
            unset_flag(&cpu->af.F, FLAG_N); // Deactivates substraction flag

            if ((cpu->bc.B & 0x0F) == 0x00) { // Check for half-carry (bit 3 to bit 4 carry)
                set_flag(&cpu->af.F, FLAG_H);
            } else {
                unset_flag(&cpu->af.F, FLAG_H);
            }
            cpu->cycles += 4;
            break;
        }
        case 0x05:{ // DEC B
            uint8_t before = cpu->bc.B;
            cpu->bc.B -= 1;

            // Z flag
            if (cpu->bc.B == 0)
                set_flag(&cpu->af.F, FLAG_Z);
            else
                unset_flag(&cpu->af.F, FLAG_Z);

            // N flag (always set for DEC)
            set_flag(&cpu->af.F, FLAG_N);

            // H flag (half-carry if borrow from bit 4)
            if ((before & 0x0F) == 0x00)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);

            cpu->cycles += 4;
            break;
        }
        case 0x06:{ // LD B, n8. Load immediate 8-bit value into B
            cpu->bc.B = memory[cpu->pc++];
            cpu->cycles += 8;
            break;
        }
        case 0x07:{ // RLCA. Rotate register A left.
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
            break;
        }
        case 0x08:{ // LD [a16], SP . Load SP into immediate 16-bit value
            uint8_t low = memory[cpu->pc++];
            uint8_t high = memory[cpu->pc++];
            uint16_t addr = (high << 8) | low;

            memory[addr] = cpu->sp & 0xFF; // Low
            memory[addr + 1] = (cpu->sp >> 8); // High
            cpu->cycles += 20;
            break;
        }
        case 0x09:{ // ADD HL, BC . Add the value in BC to HL.
            uint32_t result = cpu->hl.HL + cpu->bc.BC;

            // Flags
            unset_flag(&cpu->af.F, 6); // N = 0

            // Half Carry: if carry from bit 11
            if ((cpu->hl.HL & 0x0FFF) + (cpu->bc.BC & 0x0FFF) > 0x0FFF)
                set_flag(&cpu->af.F, 5); // H
            else
                unset_flag(&cpu->af.F, 5);

            // Carry: if carry from bit 15
            if (result > 0xFFFF)
                set_flag(&cpu->af.F, 4); // C
            else
                unset_flag(&cpu->af.F, 4);

            cpu->hl.HL = result & 0xFFFF;
            cpu->cycles += 8;
            break;
        }
        case 0x0A:{ // LD A, [BC]. Copy the value pointed to by HL into register r8.
            cpu->af.A = memory[cpu->bc.BC];
            cpu->cycles += 8;
            break;
        }
        case 0x0B:{ // DEC BC. Decrement the value in register r16 by 1.
            cpu->bc.BC -= 1;
            cpu->cycles += 8;
            break;
        }
        case 0x0C:{ // INC C. Increment the value in register r8 by 1.
            uint8_t result = cpu->bc.C += 1;
            // Zero flag
            if (result == 0){
                set_flag(&cpu->af.F, FLAG_Z);
            }
            else
                unset_flag(&cpu->af.F, FLAG_Z);
            // Clear Substract Flag
            unset_flag(&cpu->af.F, FLAG_N);
            // Half Carry
            if ((cpu->bc.C & 0x0F) + 1 > 0x0F)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);
            cpu->bc.C = result;
            cpu->cycles += 4;
            break;
        }
        case 0x0D:{ // DEC C. Decrement the value in register r8 by 1.
            uint8_t result = cpu->bc.C -= 1;
            // Zero flag
            if (result == 0){
                set_flag(&cpu->af.F, FLAG_Z);
            }
            else
                unset_flag(&cpu->af.F, FLAG_Z);
            // Enable Substract Flag
            set_flag(&cpu->af.F, FLAG_N);
            // Half Carry
            if ((cpu->bc.C & 0x0F) == 0x00)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);
            cpu->bc.C = result;
            cpu->cycles += 4;
            break;
        }
        case 0x0E:{ // LD C, n8. Copy the value n8 into register C.
            cpu->bc.C = memory[cpu->pc++];
            cpu->cycles += 8;
            break;
        }
        case 0x0F:{ //  RRCA. Rotate register A right.
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
            break;
        }
        case 0x10:{ // STOP n8 . Weird instruction... TODO
            printf("STOP instruction not yet implemented\n");
            cpu->cycles += 4;
            break;
        }
        case 0x11:{ // LD DE, n16. Load n16 into register DE
            uint8_t low = memory[cpu->pc++];
            uint8_t high = memory[cpu->pc++];
            uint16_t value = (high << 8) | low;
            cpu->de.DE = value;
            cpu->cycles += 12;
            break;
        }
        case 0x12:{ // LD [DE], A. Load A into adrees marked by [DE]
            memory[cpu->de.DE] = cpu->af.A;
            cpu->cycles += 8;
            break;
        }
        case 0x13:{ // INC DE. Increment DE by 1.
            cpu->de.DE += 1;
            cpu->cycles += 8;
            break;
        }
        case 0x14:{// INC D. Increment D by 1.
            uint8_t result = cpu->de.D + 1;
            // Zero Flag
            if (result == 0){
                set_flag(&cpu->af.F, FLAG_Z);
            }else{
                unset_flag(&cpu->af.F, FLAG_Z);
            }
            // Substraction Flag
            unset_flag(&cpu->af.F, FLAG_N);
            // Half carry flag
            if ((cpu->bc.C & 0x0F) + 1 > 0x0F)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);
            cpu->cycles += 4;
            break;
        }

        case 0x3E: // LD A, d8
            cpu->af.A = memory[cpu->pc++];
            break;

        case 0xE0: // TODO: Implement
            break;

        case 0xCB: {
            uint8_t cb_opcode = memory[cpu->pc++];
            switch (cb_opcode) {
                case 0x11: // RL C
                    // TODO: rotate C left through carry
                    break;
            }
            break;
        }

        default:
            printf("Opcode 0x%02X not implemented\n", opcode);
            break;
    }
}
