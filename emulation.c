#include "emulation.h"
void emulate_cycle(uint8_t* memory, CPU* cpu) {
    uint8_t opcode = memory[cpu->pc++];

    switch (opcode) {
        case 0x00: // NOP
            cpu->cycles += 4;
            break;
        case 0x01: { // LD BC, d16. Copies value from BC to d16
            load_r16_n16(&cpu->bc.BC, memory, cpu);
            break;
        }
        case 0x02: { // LD (BC), A. Stores A into the memory at address BC
            load_p16_n8(&cpu->bc.BC, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x03: { // INC BC. Increments BC by 1
            inc_r16(&cpu->bc.BC, memory, cpu);
            break;
        }
        case 0x04: { // INC B. Increments B by 1
            inc_r8(&cpu->bc.B, memory, cpu);
            break;
        }
        case 0x05:{ // DEC B. Decrements B by 1
            dec_r8(&cpu->bc.B, memory, cpu);
            break;
        }
        case 0x06:{ // LD B, n8. Load immediate 8-bit value into B
            load_r8_n8(&cpu->bc.B, memory, cpu);
            break;
        }
        case 0x07:{ // RLCA. Rotate register A left.
            rlca(memory, cpu);
            break;
        }
        case 0x08:{ // LD [a16], SP . Load SP into immediate 16-bit value
            load_a16_SP(memory, cpu);
            break;
        }
        case 0x09:{ // ADD HL, BC . Add the value in BC to HL.
            add_r16_r16(&cpu->hl.HL, &cpu->bc.BC, memory, cpu);
            break;
        }
        case 0x0A:{ // LD A, [BC]. Copy the value pointed to by HL into register r8.
            cpu->af.A = memory[cpu->bc.BC];
            cpu->cycles += 8;
            load_r8_p16(&cpu->af.A, &cpu->bc.BC, memory, cpu);
            break;
        }
        case 0x0B:{ // DEC BC. Decrement the value in register r16 by 1.
            dec_r16(&cpu->bc.BC, memory, cpu);
            break;
        }
        case 0x0C: { // INC C. Increment the value in register r8 by 1.
            inc_r8(&cpu->bc.C, memory, cpu);
            break;
        }
        case 0x0D: { // DEC C. Decrement the value in register r8 by 1.
            dec_r16(&cpu->bc.BC, memory, cpu);
            break;
        }

        case 0x0E:{ // LD C, n8. Copy the value n8 into register C.
            load_r8_n8(&cpu->bc.C, memory, cpu);
            break;
        }
        case 0x0F:{ //  RRCA. Rotate register A right.
            rrca(memory, cpu);
            break;
        }
        case 0x10:{ // STOP n8 . Weird instruction... TODO
            printf("STOP instruction not yet implemented\n");
            cpu->cycles += 4;
            break;
        }
        case 0x11:{ // LD DE, n16. Load n16 into register DE
            load_r16_n16(&cpu->de.DE, memory, cpu);
            break;
        }
        case 0x12:{ // LD [DE], A. Load A into adrees marked by [DE]
            load_p16_n8(&cpu->de.DE, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x13:{ // INC DE. Increment DE by 1.
            inc_r16(&cpu->de.DE, memory, cpu);
            break;
        }
        case 0x14: { // INC D. Increments D by 1.
            inc_r8(&cpu->de.D, memory, cpu);
            break;
        }
        case 0x15: { // DEC D. Decrements D by 1.
            dec_r8(&cpu->de.D, memory, cpu);
            break;
        }
        case 0x16: { // LD D, n8. Loads n8 into D register.
            load_r8_n8(&cpu->de.D, memory, cpu);
            break;
        }
        case 0x17: { // RLA. Moves to the left and saves bit 7 in carry
            rla(memory, cpu);
            break;
        }
        case 0x18: { // JR e8. Relative Jump to address n16 (encoded as a signed 8 bit)
            jump_register_e8(memory, cpu);
            break;
        }
        case 0x19: { // ADD HL, DE . Sum DE to HL
            add_r16_r16(&cpu->hl.HL, &cpu->de.DE, memory, cpu);
            break;
        }
        case 0x1A: { // LD A, [DE]. Copy the byte pointed to by DE into register A.
            load_r8_p16(&cpu->af.A, &cpu->de.DE, memory, cpu);
            break;
        }
        case 0x1B: { // DEC DE. Decrements DE by 1
            dec_r16(&cpu->de.DE, memory, cpu);
            break;
        }
        case 0x1C: { // INC E. Increments E by 1
            inc_r8(&cpu->de.E, memory, cpu);
            break;
        }
        case 0x1D: { // DEC E. Decrements E by 1
            dec_r8(&cpu->de.E, memory, cpu);
            break;
        }
        case 0x1E: { // LD E, n8. Copies n8 to E
            load_r8_n8(&cpu->de.E, memory, cpu);
            break;
        }
        case 0x1F: { // RRA. Rotate A right through carry
            rra(memory, cpu);
            break;
        }
        case 0x20:{ //JR NZ, e8. Jump relative to address +e8 if condition Z = 0 is met.
            jump_register_nz_e8(memory, cpu);
            break;
        }
        case 0x21:{ // LD HL, n16. Copy the value n16 into register HL.
            load_r16_n16(&cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x22:{ // LD [HL+], A. Copy the value in register A into the byte pointed by HL and increment HL afterwards.
            load_p16_n8_plus(&cpu->hl.HL, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x23:{// INC HL. Increment the value in register HL by 1.
            inc_r16(&cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x24:{// INC H. Increment the value in register H by 1.
            inc_r8(&cpu->af.A, memory, cpu);
            break;
        }

        default:
            printf("Opcode 0x%02X not implemented\n", opcode);
            break;
    }
}
