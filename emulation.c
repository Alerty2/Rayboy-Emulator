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
            load_p16_r8(&cpu->bc.BC, &cpu->af.A, memory, cpu);
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
            load_p16_r8(&cpu->de.DE, &cpu->af.A, memory, cpu);
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
            load_p16_r8_plus(&cpu->hl.HL, &cpu->af.A, memory, cpu);
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
        case 0x25:{// DEC H . Decrement the value in register H by 1.
            dec_r8(&cpu->hl.H, memory, cpu);
            break;
        }
        case 0x26:{// LD H, n8. Copy the value n8 into H
            load_r8_n8(&cpu->hl.H, memory, cpu);
            break;
        }
        case 0x27:{// DAA.
            dda(memory, cpu); // TODO
            break;
        }
        case 0x28:{// JR Z, e8. Jump relative +e8 if Z = 1
            jump_register_z_e8(memory, cpu);
            break;
        }
        case 0x29:{// ADD HL, HL. Sum HL to HL.
            add_r16_r16(&cpu->hl.HL, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x2A:{// LD A, [HL+]. Copy from adress HL to A and sum 1
            load_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x2B:{// DEC HL. Decrements register HL by 1
            dec_r16(&cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x2C:{// INC A. Increments register A by 1
            inc_r8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0x2D:{// DEC A. Decrements register A by 1
            dec_r8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0x2E:{// LD A, n8. Copies n8 number into A.
            load_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0x2F:{// CPL. Bitwise not. 1001011 -> 0110100
            load_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0x30:{// JR NC, e8. Jump relative +e8 if flag C == 0
            jump_register_nc_e8(memory, cpu);
            break;
        }
        case 0x31:{// LD SP, n16. Load 16 bit number into SP.
            load_r16_n16(&cpu->sp, memory, cpu);
            break;
        }
        case 0x32:{// LD [HL-], A . Copy A into HL and then rest 1
            load_p16_r8_minus(&cpu->hl.HL, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x33:{// INC SP. Increment SP by 1
            inc_r16(&cpu->sp, memory, cpu);
            break;
        }
        case 0x34:{// INC [HL] . Increment register pointed by HL by 1
            inc_p16(&cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x35:{// DEC [HL]. Decrements register pointed by HL by 1
            dec_p16(&cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x36:{// LD [HL], n8. Copies n8 into register pointed by HL
            load_p16_n8(&cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x37:{// SCF. Set Carry Flag
            set_flag(&cpu->af.F, FLAG_C);
            unset_flag(&cpu->af.F, FLAG_N);
            unset_flag(&cpu->af.F, FLAG_H);
            break;
        }
        case 0x38:{// JR C, e8. Jump relative -e8 if Flag C = 1
            set_flag(&cpu->af.F, FLAG_C);
            break;
        }
        case 0x39:{// ADD HL, SP. Sum registers HL and SP
            add_r16_r16(&cpu->hl.HL, &cpu->sp, memory, cpu);
            break;
        }
        case 0x3A:{// LD A, [HL-]. Copy the value pointed by HL to A and rest 1
            load_r8_p16_minus(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x3B:{// DEC SP. Decrement SP by 1
            dec_r16(&cpu->sp, memory, cpu);
            break;
        }
        case 0x3C:{// INC A. Increment A by 1.
            inc_r8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0x3D:{// DEC A. Decrement A by 1.
            dec_r8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0x3E:{// LD A, n8. Load n8 value into A
            load_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0x3F:{// CCF. Invert Carry Flag
            if (cpu->af.F & FLAG_C){
                unset_flag(&cpu->af.F, FLAG_C);
            }else{
                set_flag(&cpu->af.F, FLAG_C);
            }
            unset_flag(&cpu->af.F, FLAG_N);
            unset_flag(&cpu->af.F, FLAG_H);
            break;
        }
        case 0x40:{// LD B, B. copy B into B?
            load_r8_r8(&cpu->bc.B, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x41:{// LD B, C . Copy C into B
            load_r8_r8(&cpu->bc.B, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x42:{// LD B, D . Copy D into B
            load_r8_r8(&cpu->bc.B, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x43:{// LD B, E . Copy E into B
            load_r8_r8(&cpu->bc.B, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x44:{// LD B, H . Copy H into B
            load_r8_r8(&cpu->bc.B, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x45:{// LD B, L . Copy L into B
            load_r8_r8(&cpu->bc.B, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x46:{// LD B, [HL] . Copy direction pointed by HL into B
            load_r8_p16(&cpu->bc.B, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x47:{// LD B, A . Copy A into B
            load_r8_r8(&cpu->bc.B, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x48:{// LD C, B . Copy B into C
            load_r8_r8(&cpu->bc.C, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x49:{// LD C, C . Copy C into C?
            load_r8_r8(&cpu->bc.C, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x4A:{// LD C, D . Copy D into C
            load_r8_r8(&cpu->bc.C, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x4B:{// LD C, E . Copy E into C
            load_r8_r8(&cpu->bc.C, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x4C:{// LD C, H . Copy H into C
            load_r8_r8(&cpu->bc.C, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x4D:{// LD C, D . Copy D into C
            load_r8_r8(&cpu->bc.C, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x4E:{// LD C, [HL] . Copy direction pointed in HL into C
            load_r8_p16(&cpu->bc.C, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x4F:{// LD C, A . Copy A into C
            load_r8_r8(&cpu->bc.C, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x50:{// LD D, B . Copy B into D
            load_r8_r8(&cpu->de.D, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x51:{// LD D, C . Copy C into D
            load_r8_r8(&cpu->de.D, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x52:{// LD D, D . Copy D into D?
            load_r8_r8(&cpu->de.D, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x53:{// LD D, E . Copy E into D
            load_r8_r8(&cpu->de.D, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x54:{// LD D, H . Copy H into D
            load_r8_r8(&cpu->de.D, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x55:{// LD D, L . Copy L into D
            load_r8_r8(&cpu->de.D, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x56:{// LD D, [HL] . Copy value pointed by HL into D
            load_r8_p16(&cpu->de.D, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x57:{// LD D, A . Copy A into D
            load_r8_r8(&cpu->de.D, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x58:{// LD E, B . Copy B into E
            load_r8_r8(&cpu->de.E, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x59:{// LD E, C . Copy C into E
            load_r8_r8(&cpu->de.E, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x5A:{// LD E, D . Copy D into E
            load_r8_r8(&cpu->de.E, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x5B:{// LD E, E. Copy E into E
            load_r8_r8(&cpu->de.E, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x5C:{// LD E, H. Copy H into E
            load_r8_r8(&cpu->hl.H, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x5D:{// LD E, L. Copy L into E
            load_r8_r8(&cpu->hl.L, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x5E:{// LD E, [HL]. Copy value pointed by HL into E
            load_r8_p16(&cpu->de.E, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x5F:{// LD E, A. Copy A into E.
            load_r8_r8(&cpu->de.E, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x60:{// LD H, B. Copy B into H.
            load_r8_r8(&cpu->hl.H, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x61:{// LD H, C . Copy C into H
            load_r8_r8(&cpu->hl.H, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x62:{// LD H, D. Copy D into H.
            load_r8_r8(&cpu->hl.H, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x63:{// LD H, E. Copy E into H.
            load_r8_r8(&cpu->hl.H, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x64:{// LD H, H. Copy H into H?
            load_r8_r8(&cpu->hl.H, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x65:{// LD H, L. Copy L into H
            load_r8_r8(&cpu->hl.L, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x66:{// LD H, [HL]. Copy value pointed in HL into H
            load_r8_p16(&cpu->hl.L, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x67:{//LD H, A. Copy A into H
            load_r8_r8(&cpu->hl.H, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x68:{// LD L, B. Copy B into L
            load_r8_r8(&cpu->hl.L, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x69:{// LD L, C. Copy C into L
            load_r8_r8(&cpu->hl.L, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x6A:{//  LD L, D. Copy D into L
            load_r8_r8(&cpu->hl.L, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x6B:{// LD L, E. Copy E into L
            load_r8_r8(&cpu->hl.L, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x6C:{// LD L, H. Copy H into L
            load_r8_r8(&cpu->hl.L, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x6D:{// LD L, L. Copy L into L?
            load_r8_r8(&cpu->hl.L, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x6E:{// LD L, [HL]. Copy value pointed by HL into L
            load_r8_p16(&cpu->hl.L, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x6F:{// LD L, A. Copy value pointed by A into L
            load_r8_r8(&cpu->hl.L, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x70:{// LD [HL], B. Copy B into value pointed by HL.
            load_p16_r8(&cpu->hl.HL, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x71:{// LD [HL], C. Copy C into value pointed by HL.
            load_p16_r8(&cpu->hl.HL, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x72:{// LD [HL], D. Copy D into value pointed by HL.
            load_p16_r8(&cpu->hl.HL, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x73:{// LD [HL], E. Copy E into value pointed by HL.
            load_p16_r8(&cpu->hl.HL, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x74:{// LD [HL], H. Copy H into value pointed by HL.
            load_p16_r8(&cpu->hl.HL, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x75:{// LD [HL], L. Copy L into value pointed by HL.
            load_p16_r8(&cpu->hl.HL, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x76:{// HALT. TODO

            break;
        }
        case 0x77:{// LD [HL], A. Copy A into value pointed by HL.
            load_p16_r8(&cpu->hl.HL, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x78:{// LD A, B. Copy B into A.
            load_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x79:{// LD A, C. Copy C into A.
            load_r8_r8(&cpu->af.A, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x7A:{// LD A, D. Copy D into A.
            load_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x7B:{// LD A, E. Copy E into A.
            load_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x7C:{// LD A, H. Copy H into A.
            load_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x7D:{// LD A, L. Copy L into A.
            load_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x7E:{// LD A, [HL] . Copy value pointed in HL into A.
            load_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x7F:{// LD A, A. Copy value pointed in A into A?
            load_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }



        default:
            printf("Opcode 0x%02X not implemented\n", opcode);
            break;
    }
}
