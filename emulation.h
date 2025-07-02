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
        case 0x0C: { // INC C. Increment the value in register r8 by 1.
            uint8_t before = cpu->bc.C;
            uint8_t result = before + 1;
            cpu->bc.C = result;

            // Zero flag
            if (result == 0)
                set_flag(&cpu->af.F, FLAG_Z);
            else
                unset_flag(&cpu->af.F, FLAG_Z);

            // Clear Subtract flag
            unset_flag(&cpu->af.F, FLAG_N);

            // Half Carry: Set if there is a carry from bit 3 to bit 4
            if ((before & 0x0F) + 1 > 0x0F)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);

            cpu->cycles += 4;
            break;
        }

        case 0x0D: { // DEC C. Decrement the value in register r8 by 1.
            uint8_t before = cpu->bc.C;
            uint8_t result = before - 1;
            cpu->bc.C = result;

            // Zero flag
            if (result == 0)
                set_flag(&cpu->af.F, FLAG_Z);
            else
                unset_flag(&cpu->af.F, FLAG_Z);

            // Set Subtract flag (because it's a subtraction)
            set_flag(&cpu->af.F, FLAG_N);

            // Half Carry: set if borrow from bit 4
            if ((before & 0x0F) == 0x00)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);

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
        case 0x14: { // INC D. Increments D by 1.
            uint8_t result = cpu->de.D + 1;
            cpu->de.D = result;

            // Zero flag
            if (result == 0)
                set_flag(&cpu->af.F, FLAG_Z);
            else
                unset_flag(&cpu->af.F, FLAG_Z);

            // Subtraction flag
            unset_flag(&cpu->af.F, FLAG_N);

            // Half Carry flag
            if ((cpu->de.D & 0x0F) == 0x00)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);

            cpu->cycles += 4;
            break;
        }
        case 0x15: { // DEC D. Decrements D by 1.
            uint8_t result = cpu->de.D - 1;

            cpu->de.D = result;

            // Zero flag
            if (result == 0)
                set_flag(&cpu->af.F, FLAG_Z);
            else
                unset_flag(&cpu->af.F, FLAG_Z);

            // Subtraction flag
            set_flag(&cpu->af.F, FLAG_N);

            // Half Carry flag
            if ((cpu->de.D & 0x0F) == 0x0F)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);

            cpu->cycles += 4;
            break;
        }
        case 0x16: { // LD D, n8. Loads n8 into D register.
            cpu->de.D = memory[cpu->pc++];
            cpu->cycles += 8;
            break;
        }
        case 0x17: { // RLA. Moves to the left and saves bit 7 in carry
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
            break;
        }
        case 0x18: { // JR e8. Relative Jump to address n16 (encoded as a signed 8 bit)
            cpu->pc = cpu-> pc += (int8_t)memory[cpu->pc++];
            cpu->cycles += 12;
            break;
        }
        case 0x19: { // ADD HL, DE . Sum DE to HL
            uint32_t result = cpu->hl.HL + cpu->de.DE;

            // Half carry
            if (((cpu->hl.HL & 0x0FFF) + (cpu->de.DE & 0x0FFF)) > 0x0FFF)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);

            // Carry flag
            if (result > 0xFFFF)
                set_flag(&cpu->af.F, FLAG_C);
            else
                unset_flag(&cpu->af.F, FLAG_C);

            unset_flag(&cpu->af.F, FLAG_N); // Reset N flag

            cpu->hl.HL = (uint16_t)result;
            cpu->cycles += 8;
            break;
        }
        case 0x1A: { // LD A, [DE]. Copy the byte pointed to by DE into register A.
            cpu->af.A = memory[cpu->de.DE];
            cpu->cycles += 8;
            break;
        }
        case 0x1B: { // DEC DE. Decrements DE by 1
            cpu->de.DE -= 1;
            cpu->cycles += 8;
            break;
        }
        case 0x1C: { // INC E. Increments E by 1
            uint8_t result = cpu->de.E + 1;

            // Zero flag
            if (result == 0)
                set_flag(&cpu->af.F, FLAG_Z);
            else
                unset_flag(&cpu->af.F, FLAG_Z);

            // Clear Subtract flag (N)
            unset_flag(&cpu->af.F, FLAG_N);

            // Half Carry flag: set if carry from bit 3 to bit 4
            if ((cpu->de.E & 0x0F) + 1 > 0x0F)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);

            cpu->de.E = result;
            cpu->cycles += 4;
            break;
        }
        case 0x1D: { // DEC E. Decrements E by 1
            uint8_t before = cpu->de.E;
            uint8_t result = before - 1;
            cpu->de.E = result;

            // Zero flag
            if (result == 0)
                set_flag(&cpu->af.F, FLAG_Z);
            else
                unset_flag(&cpu->af.F, FLAG_Z);

            // Set Subtract flag (because it's a subtraction)
            set_flag(&cpu->af.F, FLAG_N);

            // Half Carry: set if borrow from bit 4
            if ((before & 0x0F) == 0x00)
                set_flag(&cpu->af.F, FLAG_H);
            else
                unset_flag(&cpu->af.F, FLAG_H);

            cpu->cycles += 4;
            break;
        }
        case 0x1E: { // LD E, n8. Copies n8 to E
            cpu->de.E = memory[cpu->pc++];
            cpu->cycles += 8;
            break;
        }
        case 0x1F: { // RRA. Rotate A right through carry
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
