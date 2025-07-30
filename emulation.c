#include "emulation.h"
void handle_interrupts(uint8_t* memory, CPU* cpu) {
    if (!cpu->ime) return;

    uint8_t IE = memory[0xFFFF]; // Interrupt Enable
    uint8_t IF = memory[0xFF0F]; // Interrupt Flag

    uint8_t fired = IE & IF;
    if (!fired) return;


    for (int i = 0; i < 5; i++) {
        if (fired & (1 << i)) {
            cpu->ime = 0;
            memory[0xFF0F] &= ~(1 << i);

            // Push PC to stack
            cpu->sp -= 2;
            memory[cpu->sp] = cpu->pc & 0xFF;
            memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;

            switch (i) {
                case 0: cpu->pc = 0x40; break; // VBlank
                case 1: cpu->pc = 0x48; break; // LCD STAT
                case 2: cpu->pc = 0x50; break; // Timer
                case 3: cpu->pc = 0x58; break; // Serial
                case 4: cpu->pc = 0x60; break; // Joypad
            }

            cpu->cycles += 20;
            return;
        }
    }
}

int emulate_cycle(uint8_t* memory, CPU* cpu) {
    if (cpu->pc >= 0x10000) {
        printf("Invalid PC: %04X\n", cpu->pc);
        exit(1);
    }
    printf("[PC=%04X] Opcode=%02X\n", cpu->pc, memory[cpu->pc]);


    uint8_t opcode = memory[cpu->pc++];
    uint8_t ie = memory[0xFFFF];  // Interrupt Enable
    uint8_t iflag = memory[0xFF0F]; // Interrupt Flag

    uint8_t interrupt_bits = ie & iflag;

    if (cpu->ime && interrupt_bits) {
        // Pending interruption
        for (int i = 0; i < 5; i++) {
            if (interrupt_bits & (1 << i)) {
                // Desactivates IME
                cpu->ime = false;

                // Cleans bit of interruption
                memory[0xFF0F] &= ~(1 << i);

                // Pushes PC to stack
                cpu->sp -= 2;
                memory[cpu->sp] = cpu->pc & 0xFF;
                memory[cpu->sp + 1] = cpu->pc >> 8;

                // Salta a la dirección de la interrupción
                static const uint16_t interrupt_vector[5] = {
                    0x40, 0x48, 0x50, 0x58, 0x60
                };
                cpu->pc = interrupt_vector[i];

                cpu->cycles += 20; // Tiempo típico de interrupción

                return 0; // Salimos de este ciclo, ya hemos manejado una interrupción
            }
        }
    }

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
        case 0x80:{// ADD A, B. Sum values A and B
            add_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x81:{// ADD A, C. Sum values A and C
            add_r8_r8(&cpu->af.A, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x82:{// ADD A, D. Sum values A and D
            add_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x83:{// ADD A, E. Sum values A and E
            add_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x84:{// ADD A, H. Sum values A and H
            add_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x85:{// ADD A, L. Sum values A and L
            add_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x86:{// ADD A, [HL]. Sum values A and value pointed by HL
            add_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x87:{// ADD A, A. Sum values A and A
            add_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x88:{// ADC A, B. Sum values A and B and also the carry flag.
            adc_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x89:{// ADC A, C. Sum values A and C and also the carry flag.
            adc_r8_r8(&cpu->af.A, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x8A:{// ADC A, D. Sum values A and D and also the carry flag.
            adc_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x8B:{// ADC A, E. Sum values A and E and also the carry flag.
            adc_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x8C:{// ADC A, H. Sum values A and H and also the carry flag.
            adc_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x8D:{// ADC A, L. Sum values A and L and also the carry flag.
            adc_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x8E:{// ADC A, [HL]. Sum values A and value pointed by HL and also the carry flag.
            adc_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x8F:{// ADC A, A. Sum values A and A and also the carry flag.
            adc_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x90:{// SUB A, B. Rest values A and B.
            sub_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x91:{// SUB A, C. Rest values A and C.
            sub_r8_r8(&cpu->af.A, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x92:{// SUB A, D. Rest values A and D.
            sub_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x93:{// SUB A, E. Rest values A and E.
            sub_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x94:{// SUB A, H. Rest values A and H.
            sub_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x95:{// SUB A, L. Rest values A and L.
            sub_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x96:{// SUB A, [HL]. Rest values A and value pointed by HL.
            sub_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x97:{// SUB A, A. Rest values A and A
            sub_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0x98:{// SBC A, B. Rest values A and B and Carry
            sbc_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0x99:{// SBC A, C. Rest values A and C and Carry
            sbc_r8_r8(&cpu->af.A, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0x9A:{// SBC A, D. Rest values A and D and Carry
            sbc_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0x9B:{// SBC A, E. Rest values A and E and Carry
            sbc_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0x9C:{// SBC A, H. Rest values A and H and Carry
            sbc_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0x9D:{// SBC A, L. Rest values A and L and Carry
            sbc_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0x9E:{// SBC A, [HL]. Rest values A and value pointed by HL and Carry
            sbc_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0x9F:{// SBC A, A. Rest values A and A and Carry
            sbc_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0xA0:{// AND A, B. Bitwise AND between A and C.
            and_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0xA1:{// AND A, C. Bitwise AND between A and C.
            and_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0xA2:{// AND A, D. Bitwise AND between A and D.
            and_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0xA3:{// AND A, E. Bitwise AND between A and E.
            and_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0xA4:{// AND A, H. Bitwise AND between A and H.
            and_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0xA5:{// AND A, L. Bitwise AND between A and L.
            and_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0xA6:{// AND A, [HL]. Bitwise AND between A and value pointed by HL
            and_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0xA7:{// AND A, A. Bitwise AND between A and A
            and_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0xA8:{// XOR A, B. Bitwise XOR between A and B
            xor_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0xA9:{// XOR A, C. Bitwise XOR between A and C
            xor_r8_r8(&cpu->af.A, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0xAA:{// XOR A, D. Bitwise XOR between A and D
            xor_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0xAB:{// XOR A, E. Bitwise XOR between A and E
            xor_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0xAC:{// XOR A, H. Bitwise XOR between A and H
            xor_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0xAD:{// XOR A, L. Bitwise XOR between A and L
            xor_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0xAE:{// XOR A, [HL]. Bitwise XOR between A and value pointed by HL
            xor_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0xAF:{// XOR A, A. Bitwise XOR between A and A
            xor_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0xB0:{// OR A, B. Bitwise OR between A and B
            or_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0xB1:{// OR A, C. Bitwise OR between A and C
            or_r8_r8(&cpu->af.A, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0xB2:{// OR A, D. Bitwise OR between A and D
            or_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0xB3:{// OR A, E. Bitwise OR between A and E
            or_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0xB4:{// OR A, H. Bitwise OR between A and H
            or_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0xB5:{// OR A, L. Bitwise OR between A and L
            or_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0xB6:{// OR A, [HL]. Bitwise OR between A and value pointed by HL
            or_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0xB7:{// OR A, A. Bitwise OR between A and A
            or_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0xB8:{// CP A, B. Compare A and B and put flags accoordingly.
            cp_r8_r8(&cpu->af.A, &cpu->bc.B, memory, cpu);
            break;
        }
        case 0xB9:{// CP A, C. Compare A and C and put flags accoordingly.
            cp_r8_r8(&cpu->af.A, &cpu->bc.C, memory, cpu);
            break;
        }
        case 0xBA:{// CP A, D. Compare A and D and put flags accoordingly.
            cp_r8_r8(&cpu->af.A, &cpu->de.D, memory, cpu);
            break;
        }
        case 0xBB:{// CP A, E. Compare A and E and put flags accoordingly.
            cp_r8_r8(&cpu->af.A, &cpu->de.E, memory, cpu);
            break;
        }
        case 0xBC:{// CP A, H. Compare A and H and put flags accoordingly.
            cp_r8_r8(&cpu->af.A, &cpu->hl.H, memory, cpu);
            break;
        }
        case 0xBD:{// CP A, L. Compare A and L and put flags accoordingly.
            cp_r8_r8(&cpu->af.A, &cpu->hl.L, memory, cpu);
            break;
        }
        case 0xBE:{// CP A, [HL]. Compare A and value pointed by HL and put flags accoordingly.
            cp_r8_p16(&cpu->af.A, &cpu->hl.HL, memory, cpu);
            break;
        }
        case 0xBF:{// CP A, A. Compare A and A and put flags accoordingly.
            cp_r8_r8(&cpu->af.A, &cpu->af.A, memory, cpu);
            break;
        }
        case 0xC0:{// RET NZ. Goes to adress saved in sp (stack) if FLAG ZERO = 0
            return_nz(memory, cpu);
            break;
        }
        case 0xC1:{// POP BC. Copies into BC the value stored in the adress pointed by SP.
            pop_r16(&cpu->bc.BC,memory, cpu);
            break;
        }
        case 0xC2:{// JP NZ, a16. Jumps into a16 if FLAG ZERO = 0
            jump_pointer_nz_a16(memory, cpu);
            break;
        }
        case 0xC3:{// JP a16. Jumps into a16.
            jump_pointer_a16(memory, cpu);
            break;
        }
        case 0xC4:{// CALL NZ, a16. Copy current pc to stack and go to a16 if FLAG ZERO = 0
            call_nz_a16(memory, cpu);
            break;
        }
        case 0xC5:{// PUSH BC. Copy BC to stack.
            push_r16(&cpu->bc.BC, memory, cpu);
            break;
        }
        case 0xC6:{// ADD A, n8. Sum register A and n8
            add_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xC7:{// RST $00. Copy current pc to stack and go to vector (0x00)
            rst_vec(0x00, memory, cpu);
            break;
        }
        case 0xC8:{// RET Z. Go to address saved in stack if FLAG ZERO = 1
            return_z(memory, cpu);
            break;
        }
        case 0xC9:{// RET. Go to address saved in stack.
            return_(memory, cpu);
            break;
        }
        case 0xCA:{// JP Z, a16. Jump to a16 if FLAG ZERO = 1
            jump_pointer_z_a16(memory, cpu);
            break;
        }
        case 0xCB:{// PREFIX. Makes different binary operations depending on the parameter
            uint8_t cb_opcode = memory[cpu->pc++];
            switch (cb_opcode) {
                case 0x00:{ // RLC B. Rotate byte to left
                    rlc_r8(&cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x01:{ // RLC C. Rotate byte to left
                    rlc_r8(&cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x02:{ // RLC D. Rotate byte to left
                    rlc_r8(&cpu->de.D, memory, cpu);
                    break;
                }
                case 0x03:{ // RLC E. Rotate byte to left
                    rlc_r8(&cpu->de.E, memory, cpu);
                    break;
                }
                case 0x04:{ // RLC H. Rotate byte to left
                    rlc_r8(&cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x05:{ // RLC L. Rotate byte to left
                    rlc_r8(&cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x06:{ // RLC [HL]. Rotate byte pointed by HL to left
                    rlc_p16(&cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x07:{ // RLC A. Rotate byte to left
                    rlc_r8(&cpu->af.A, memory, cpu);
                    break;
                }
                case 0x08:{ // RRC B. Rotate byte to right
                    rrc_r8(&cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x09:{ // RRC C. Rotate byte to right
                    rrc_r8(&cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x0A:{ // RRC D. Rotate byte to right
                    rrc_r8(&cpu->de.D, memory, cpu);
                    break;
                }
                case 0x0B:{ // RRC E. Rotate byte to right
                    rrc_r8(&cpu->de.E, memory, cpu);
                    break;
                }
                case 0x0C:{ // RRC H. Rotate byte to right
                    rrc_r8(&cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x0D:{ // RRC L. Rotate byte to right
                    rrc_r8(&cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x0E:{ // RRC [HL]. Rotate byte pointed by HL to right
                    rrc_p16(&cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x0F:{ // RRC A. Rotate byte to right
                    rrc_r8(&cpu->af.A, memory, cpu);
                    break;
                }
                case 0x10:{ // RL B. Rotate B bits to the left and bit 0 = carry
                    rl_r8(&cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x11:{ // RL C. Rotate C bits to the left and bit 0 = carry
                    rl_r8(&cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x12:{ // RL D. Rotate D bits to the left and bit 0 = carry
                    rl_r8(&cpu->de.D, memory, cpu);
                    break;
                }
                case 0x13:{ // RL E. Rotate E bits to the left and bit 0 = carry
                    rl_r8(&cpu->de.E, memory, cpu);
                    break;
                }
                case 0x14:{ // RL H. Rotate H bits to the left and bit 0 = carry
                    rl_r8(&cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x15:{ // RL L. Rotate L bits to the left and bit 0 = carry
                    rl_r8(&cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x16:{ // RL [HL]. Rotate register pointed by HL bits to the left and bit 0 = carry
                    rl_p16(&cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x17:{ // RL A. Rotate A bits to the left and bit 0 = carry
                    rl_r8(&cpu->af.A, memory, cpu);
                    break;
                }
                case 0x18:{ // RR B. Rotate B bits to the right and bit carry = 0
                    rr_r8(&cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x19:{ // RR C. Rotate C bits to the right and bit carry = 0
                    rr_r8(&cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x1A:{ // RR D. Rotate D bits to the right and bit carry = 0
                    rr_r8(&cpu->de.D, memory, cpu);
                    break;
                }
                case 0x1B:{ // RR E. Rotate E bits to the right and bit carry = 0
                    rr_r8(&cpu->de.E, memory, cpu);
                    break;
                }
                case 0x1C:{ // RR H. Rotate H bits to the right and bit carry = 0
                    rr_r8(&cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x1D:{ // RR L. Rotate L bits to the right and bit carry = 0
                    rr_r8(&cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x1E:{ // RR [HL]. Rotate register pointed by HL bits to the right and bit carry = 0
                    rr_p16(&cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x1F:{ // RR A. Rotate A bits to the right and bit carry = 0
                    rr_r8(&cpu->af.A, memory, cpu);
                    break;
                }
                case 0x20:{ // SLA B. Shift bits to the left and bit 0 = 0
                    sla_r8(&cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x21:{ // SLA C. Shift bits to the left and bit 0 = 0
                    sla_r8(&cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x22:{ // SLA D. Shift bits to the left and bit 0 = 0
                    sla_r8(&cpu->de.D, memory, cpu);
                    break;
                }
                case 0x23:{ // SLA E. Shift bits to the left and bit 0 = 0
                    sla_r8(&cpu->de.E, memory, cpu);
                    break;
                }
                case 0x24:{ // SLA H. Shift bits to the left and bit 0 = 0
                    sla_r8(&cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x25:{ // SLA L. Shift bits to the left and bit 0 = 0
                    sla_r8(&cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x26:{ // SLA [HL]. Shift bits from register pointed by HL to the left and bit 0 = 0
                    sla_p16(&cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x27:{ // SLA A. Shift bits to the left and bit 0 = 0
                    sla_r8(&cpu->af.A, memory, cpu);
                    break;
                }
                case 0x28:{ // SRA B. Shift bits to the right
                    sra_r8(&cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x29:{ // SRA C. Shift bits to the right
                    sra_r8(&cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x2A:{ // SRA D. Shift bits to the right
                    sra_r8(&cpu->de.D, memory, cpu);
                    break;
                }
                case 0x2B:{ // SRA E. Shift bits to the right
                    sra_r8(&cpu->de.E, memory, cpu);
                    break;
                }
                case 0x2C:{ // SRA H. Shift bits to the right
                    sra_r8(&cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x2D:{ // SRA L. Shift bits to the right
                    sra_r8(&cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x2E:{ // SRA [HL]. Shift register pointed by HL to the right
                    sra_p16(&cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x2F:{ // SRA A. Shift bits to the right
                    sra_r8(&cpu->af.A, memory, cpu);
                    break;
                }
                case 0x30:{ // SWAP B. Swap the upper 4 bits in register B and the lower 4 ones
                    swap_r8(&cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x31:{ // SWAP C. Swap the upper 4 bits in register C and the lower 4 ones
                    swap_r8(&cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x32:{ // SWAP D. Swap the upper 4 bits in register D and the lower 4 ones
                    swap_r8(&cpu->de.D, memory, cpu);
                    break;
                }
                case 0x33:{ // SWAP E. Swap the upper 4 bits in register E and the lower 4 ones
                    swap_r8(&cpu->de.E, memory, cpu);
                    break;
                }
                case 0x34:{ // SWAP H. Swap the upper 4 bits in register H and the lower 4 ones
                    swap_r8(&cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x35:{ // SWAP L. Swap the upper 4 bits in register L and the lower 4 ones
                    swap_r8(&cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x36:{ // SWAP [HL]. Swap the upper 4 bits in register pointed by HL and the lower 4 ones
                    swap_p16(&cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x37:{ // SWAP A. Swap the upper 4 bits in register A and the lower 4 ones
                    swap_r8(&cpu->af.A, memory, cpu);
                    break;
                }
                case 0x38:{ // SRL B. Shift bits right and bit 0 to FLAG_C
                    srl_r8(&cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x39:{ // SRL C. Shift bits right and bit 0 to FLAG_C
                    srl_r8(&cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x3A:{ // SRL D. Shift bits right and bit 0 to FLAG_C
                    srl_r8(&cpu->de.D, memory, cpu);
                    break;
                }
                case 0x3B:{ // SRL E. Shift bits right and bit 0 to FLAG_C
                    srl_r8(&cpu->de.E, memory, cpu);
                    break;
                }
                case 0x3C:{ // SRL H. Shift bits right and bit 0 to FLAG_C
                    srl_r8(&cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x3D:{ // SRL L. Shift bits right and bit 0 to FLAG_C
                    srl_r8(&cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x3E:{ // SRL [HL]. Shift bits of register pointed by HL right and bit 0 to FLAG_C
                    srl_p16(&cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x3F:{ // SRL A. Shift bits right and bit 0 to FLAG_C
                    srl_r8(&cpu->af.A, memory, cpu);
                    break;
                }
                case 0x40:{ // BIT 0, B. If bit number 0 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(0, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x41:{ // BIT 0, C. If bit number 0 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(0, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x42:{ // BIT 0, D. If bit number 0 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(0, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x43:{ // BIT 0, E. If bit number 0 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(0, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x44:{ // BIT 0, H. If bit number 0 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(0, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x45:{ // BIT 0, L. If bit number 0 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(0, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x46:{ // BIT 0, [HL]. If bit number 0 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_p16(0, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x47:{ // BIT 0, A. If bit number 0 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(0, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x48:{ // BIT 1, B. If bit number 1 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(1, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x49:{ // BIT 1, C. If bit number 1 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(1, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x4A:{ // BIT 1, D. If bit number 1 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(1, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x4B:{ // BIT 1, E. If bit number 1 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(1, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x4C:{ // BIT 1, H. If bit number 1 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(1, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x4D:{ // BIT 1, L. If bit number 1 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(1, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x4E:{ // BIT 1, [HL]. If bit number 1 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_p16(1, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x4F:{ // BIT 1, A. If bit number 1 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(1, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x50:{ // BIT 2, B. If bit number 2 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(2, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x51:{ // BIT 2, C. If bit number 2 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(2, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x52:{ // BIT 2, D. If bit number 2 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(2, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x53:{ // BIT 2, E. If bit number 2 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(2, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x54:{ // BIT 2, H. If bit number 2 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(2, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x55:{ // BIT 2, L. If bit number 2 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(2, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x56:{ // BIT 2, [HL]. If bit number 2 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_p16(2, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x57:{ // BIT 2, A. If bit number 2 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(2, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x58:{ // BIT 3, B. If bit number 3 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(3, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x59:{ // BIT 3, C. If bit number 3 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(3, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x5A:{ // BIT 3, D. If bit number 3 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(3, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x5B:{ // BIT 3, E. If bit number 3 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(3, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x5C:{ // BIT 3, H. If bit number 3 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(3, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x5D:{ // BIT 3, L .If bit number 3 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(3, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x5E:{ // BIT 3, [HL] .If bit number 3 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_p16(3, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x5F:{ // BIT 3, A .If bit number 3 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(3, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x60:{ // BIT 4, B .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(4, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x61:{ // BIT 4, C .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(4, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x62:{ // BIT 4, D .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(4, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x63:{ // BIT 4, E .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(4, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x64:{ // BIT 4, H .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(4, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x65:{ // BIT 4, L .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(4, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x66:{ // BIT 4, [HL] .If bit number 4 from address pointed by HL is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_p16(4, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x67:{ // BIT 4, A .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(4, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x68:{ // BIT 5, B .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(5, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x69:{ // BIT 5, C .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(5, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x6A:{ // BIT 5, D .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(5, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x6B:{ // BIT 5, E .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(5, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x6C:{ // BIT 5, H .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(5, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x6D:{ // BIT 5, L .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(5, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x6E:{ // BIT 5, [HL] .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_p16(5, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x6F:{ // BIT 5, A .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(5, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x70:{ // BIT 6, B .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(6, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x71:{ // BIT 6, C .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(6, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x72:{ // BIT 6, D .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(6, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x73:{ // BIT 6, E .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(6, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x74:{ // BIT 6, H .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(6, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x75:{ // BIT 6, L .If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(6, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x76:{ // BIT 6, [HL] .If bit number 4 from address pointed by HL is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_p16(6, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x77:{ // BIT 6, A.If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(6, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x78:{ // BIT 7, B.If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(7, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x79:{ // BIT 7, C.If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(7, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x7A:{ // BIT 7, D.If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(7, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x7B:{ // BIT 7, E. If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(7, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x7C:{ // BIT 7, H. If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(7, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x7D:{ // BIT 7, L. If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(7, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x7E:{ // BIT 7, [HL]. If bit number 4 from address pointed by HL is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_p16(7, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x7F:{ // BIT 7, A. If bit number 4 is activated, when 1 -> FLAG_Z = 0, when 0 -> FLAG_Z = 1
                    bit_u3_r8(7, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x80:{ // RES 0, B. Set to 0 bit number 0.
                    res_u3_r8(0, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x81:{ // RES 0, C. Set to 0 bit number 0.
                    res_u3_r8(0, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x82:{ // RES 0, D. Set to 0 bit number 0.
                    res_u3_r8(0, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x83:{ // RES 0, E. Set to 0 bit number 0.
                    res_u3_r8(0, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x84:{ // RES 0, H. Set to 0 bit number 0.
                    res_u3_r8(0, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x85:{ // RES 0, H. Set to 0 bit number 0.
                    res_u3_r8(0, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x86:{ // RES 0, [HL]. Set to 0 bit number 0 from address pointed by HL.
                    res_u3_p16(0, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x87:{ // RES 0, A. Set to 0 bit number 0.
                    res_u3_r8(0, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x88:{ // RES 1, B. Set to 0 bit number 1.
                    res_u3_r8(1, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x89:{ // RES 1, C. Set to 0 bit number 1.
                    res_u3_r8(1, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x8A:{ // RES 1, D. Set to 0 bit number 1.
                    res_u3_r8(1, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x8B:{ // RES 1, E. Set to 0 bit number 1.
                    res_u3_r8(1, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x8C:{ // RES 1, H. Set to 0 bit number 1.
                    res_u3_r8(1, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x8D:{ // RES 1, L. Set to 0 bit number 1.
                    res_u3_r8(1, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x8E:{ // RES 1, [HL]. Set to 0 bit number 1 from address pointed by HL.
                    res_u3_p16(1, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x8F:{ // RES 1, A. Set to 0 bit number 1.
                    res_u3_r8(1, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x90:{ // RES 2, B. Set to 0 bit number 2.
                    res_u3_r8(2, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x91:{ // RES 2, C. Set to 0 bit number 2.
                    res_u3_r8(2, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x92:{ // RES 2, D. Set to 0 bit number 2.
                    res_u3_r8(2, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x93:{ // RES 2, E. Set to 0 bit number 2.
                    res_u3_r8(2, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x94:{ // RES 2, H. Set to 0 bit number 2.
                    res_u3_r8(2, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x95:{ // RES 2, L. Set to 0 bit number 2.
                    res_u3_r8(2, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x96:{ // RES 2, [HL]. Set to 0 bit number 2 from address pointed by HL
                    res_u3_p16(2, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x97:{ // RES 2, A. Set to 0 bit number 2.
                    res_u3_r8(2, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0x98:{ // RES 3, B. Set to 0 bit number 3.
                    res_u3_r8(3, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0x99:{ // RES 3, C. Set to 0 bit number 3.
                    res_u3_r8(3, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0x9A:{ // RES 3, D. Set to 0 bit number 3.
                    res_u3_r8(3, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0x9B:{ // RES 3, E. Set to 0 bit number 3.
                    res_u3_r8(3, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0x9C:{ // RES 3, H. Set to 0 bit number 3.
                    res_u3_r8(3, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0x9D:{ // RES 3, H. Set to 0 bit number 3.
                    res_u3_r8(3, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0x9E:{ // RES 3, [HL]. Set to 0 bit number 3 from register pointed by HL.
                    res_u3_p16(3, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0x9F:{ // RES 3, A. Set to 0 bit number 3.
                    res_u3_r8(3, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xA0:{ // RES 4, B. Set to 0 bit number 4.
                    res_u3_r8(4, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xA1:{ // RES 4, C. Set to 0 bit number 4.
                    res_u3_r8(4, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xA2:{ // RES 4, D. Set to 0 bit number 4.
                    res_u3_r8(4, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xA3:{ // RES 4, E. Set to 0 bit number 4.
                    res_u3_r8(4, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xA4:{ // RES 4, H. Set to 0 bit number 4.
                    res_u3_r8(4, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xA5:{ // RES 4, L. Set to 0 bit number 4.
                    res_u3_r8(4, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xA6:{ // RES 4, [HL]. Set to 0 bit number 4 from address pointed by HL
                    res_u3_p16(4, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xA7:{ // RES 4, A. Set to 0 bit number 4.
                    res_u3_r8(4, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xA8:{ // RES 5, B. Set to 0 bit number 5.
                    res_u3_r8(5, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xA9:{ // RES 5, C. Set to 0 bit number 5.
                    res_u3_r8(5, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xAA:{ // RES 5, D. Set to 0 bit number 5.
                    res_u3_r8(5, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xAB:{ // RES 5, E. Set to 0 bit number 5.
                    res_u3_r8(5, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xAC:{ // RES 5, H. Set to 0 bit number 5.
                    res_u3_r8(5, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xAD:{ // RES 5, L. Set to 0 bit number 5.
                    res_u3_r8(5, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xAE:{ // RES 5, [HL]. Set to 0 bit number 5 from register pointed by HL
                    res_u3_p16(5, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xAF:{ // RES 5, F. Set to 0 bit number 5.
                    res_u3_r8(5, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xB0:{ // RES 6, B. Set to 0 bit number 6.
                    res_u3_r8(6, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xB1:{ // RES 6, C. Set to 0 bit number 6.
                    res_u3_r8(6, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xB2:{ // RES 6, D. Set to 0 bit number 6.
                    res_u3_r8(6, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xB3:{ // RES 6, E. Set to 0 bit number 6.
                    res_u3_r8(6, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xB4:{ // RES 6, H. Set to 0 bit number 6.
                    res_u3_r8(6, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xB5:{ // RES 6, L. Set to 0 bit number 6.
                    res_u3_r8(6, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xB6:{ // RES 6, [HL]. Set to 0 bit number 6 from address pointed by HL
                    res_u3_p16(6, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xB7:{ // RES 6, A. Set to 0 bit number 6.
                    res_u3_r8(6, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xB8:{ // RES 7, B. Set to 0 bit number 7.
                    res_u3_r8(7, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xB9:{ // RES 7, C. Set to 0 bit number 7.
                    res_u3_r8(7, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xBA:{ // RES 7, D. Set to 0 bit number 7.
                    res_u3_r8(7, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xBB:{ // RES 7, D. Set to 0 bit number 7.
                    res_u3_r8(7, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xBC:{ // RES 7, H. Set to 0 bit number 7.
                    res_u3_r8(7, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xBD:{ // RES 7, L. Set to 0 bit number 7.
                    res_u3_r8(7, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xBE:{ // RES 7, [HL]. Set to 0 bit number 7 from address pointed by HL
                    res_u3_p16(7, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xBF:{ // RES 7, A. Set to 0 bit number 7.
                    res_u3_r8(7, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xC0:{ // SET 0, B. Set to 1 bit number 0.
                    set_u3_r8(0, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xC1:{ // SET 0, B. Set to 1 bit number 0.
                    set_u3_r8(0, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xC2:{ // SET 0, D. Set to 1 bit number 0.
                    set_u3_r8(0, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xC3:{ // SET 0, E. Set to 1 bit number 0.
                    set_u3_r8(0, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xC4:{ // SET 0, H. Set to 1 bit number 0.
                    set_u3_r8(0, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xC5:{ // SET 0, H. Set to 1 bit number 0.
                    set_u3_r8(0, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xC6:{ // SET 0, [HL]. Set to 1 bit number 0 from adress pointed by HL
                    set_u3_p16(0, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xC7:{ // SET 0, A. Set to 1 bit number 0.
                    set_u3_r8(0, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xC8:{ // SET 1, B. Set to 1 bit number 1.
                    set_u3_r8(1, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xC9:{ // SET 1, C. Set to 1 bit number 1.
                    set_u3_r8(1, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xCA:{ // SET 1, D. Set to 1 bit number 1.
                    set_u3_r8(1, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xCB:{ // SET 1, E. Set to 1 bit number 1.
                    set_u3_r8(1, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xCC:{ // SET 1, H. Set to 1 bit number 1.
                    set_u3_r8(1, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xCD:{ // SET 1, L. Set to 1 bit number 1.
                    set_u3_r8(1, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xCE:{ // SET 1, [HL]. Set to 1 bit number 1 from addres pointed by HL
                    set_u3_p16(1, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xCF:{ // SET 1, A. Set to 1 bit number 1.
                    set_u3_r8(1, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xD0:{ // SET 2, B. Set to 1 bit number 2.
                    set_u3_r8(2, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xD1:{ // SET 2, C. Set to 1 bit number 2.
                    set_u3_r8(2, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xD2:{ // SET 2, D. Set to 1 bit number 2.
                    set_u3_r8(2, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xD3:{ // SET 2, E. Set to 1 bit number 2.
                    set_u3_r8(2, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xD4:{ // SET 2, H. Set to 1 bit number 2.
                    set_u3_r8(2, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xD5:{ // SET 2, L. Set to 1 bit number 2.
                    set_u3_r8(2, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xD6:{ // SET 2, [HL]. Set to 1 bit number 2 from address pointed by HL
                    set_u3_p16(2, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xD7:{ // SET 2, A. Set to 1 bit number 2.
                    set_u3_r8(2, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xD8:{ // SET 3, B. Set to 1 bit number 3.
                    set_u3_r8(3, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xD9:{ // SET 3, C. Set to 1 bit number 3.
                    set_u3_r8(3, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xDA:{ // SET 3, D. Set to 1 bit number 3.
                    set_u3_r8(3, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xDB:{ // SET 3, E. Set to 1 bit number 3.
                    set_u3_r8(3, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xDC:{ // SET 3, H. Set to 1 bit number 3.
                    set_u3_r8(3, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xDD:{ // SET 3, L. Set to 1 bit number 3.
                    set_u3_r8(3, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xDE:{ // SET 3, [HL]. Set to 1 bit number 3 from address pointed by HL
                    set_u3_p16(3, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xDF:{ // SET 3, A. Set to 1 bit number 3.
                    set_u3_r8(3, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xE0:{ // SET 4, B. Set to 1 bit number 4.
                    set_u3_r8(4, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xE1:{ // SET 4, C. Set to 1 bit number 4.
                    set_u3_r8(4, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xE2:{ // SET 4, D. Set to 1 bit number 4.
                    set_u3_r8(4, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xE3:{ // SET 4, E. Set to 1 bit number 4.
                    set_u3_r8(4, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xE4:{ // SET 4, H. Set to 1 bit number 4.
                    set_u3_r8(4, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xE5:{ // SET 4, L. Set to 1 bit number 4.
                    set_u3_r8(4, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xE6:{ // SET 4, [HL]. Set to 1 bit number 4 from address pointed by HL
                    set_u3_p16(4, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xE7:{ // SET 4, A. Set to 1 bit number 4.
                    set_u3_r8(4, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xE8:{ // SET 5, B. Set to 1 bit number 5.
                    set_u3_r8(5, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xE9:{ // SET 5, C. Set to 1 bit number 5.
                    set_u3_r8(5, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xEA:{ // SET 5, D. Set to 1 bit number 5.
                    set_u3_r8(5, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xEB:{ // SET 5, E. Set to 1 bit number 5.
                    set_u3_r8(5, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xEC:{ // SET 5, H. Set to 1 bit number 5.
                    set_u3_r8(5, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xED:{ // SET 5, L. Set to 1 bit number 5.
                    set_u3_r8(5, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xEE:{ // SET 5, [HL]. Set to 1 bit number 5 from address pointed by HL
                    set_u3_p16(5, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xEF:{ // SET 5, A. Set to 1 bit number 5.
                    set_u3_r8(5, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xF0:{ // SET 6, B. Set to 1 bit number 6.
                    set_u3_r8(6, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xF1:{ // SET 6, C. Set to 1 bit number 6.
                    set_u3_r8(6, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xF2:{ // SET 6, D. Set to 1 bit number 6.
                    set_u3_r8(6, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xF3:{ // SET 6, E. Set to 1 bit number 6.
                    set_u3_r8(6, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xF4:{ // SET 6, H. Set to 1 bit number 6.
                    set_u3_r8(6, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xF5:{ // SET 6, L. Set to 1 bit number 6.
                    set_u3_r8(6, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xF6:{ // SET 6, [HL]. Set to 1 bit number 6 from address pointed by HL
                    set_u3_p16(6, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xF7:{ // SET 6, A. Set to 1 bit number 6.
                    set_u3_r8(6, &cpu->af.A, memory, cpu);
                    break;
                }
                case 0xF8:{ // SET 7, B. Set to 1 bit number 7.
                    set_u3_r8(7, &cpu->bc.B, memory, cpu);
                    break;
                }
                case 0xF9:{ // SET 7, C. Set to 1 bit number 7.
                    set_u3_r8(7, &cpu->bc.C, memory, cpu);
                    break;
                }
                case 0xFA:{ // SET 7, D. Set to 1 bit number 7.
                    set_u3_r8(7, &cpu->de.D, memory, cpu);
                    break;
                }
                case 0xFB:{ // SET 7, E. Set to 1 bit number 7.
                    set_u3_r8(7, &cpu->de.E, memory, cpu);
                    break;
                }
                case 0xFC:{ // SET 7, H. Set to 1 bit number 7.
                    set_u3_r8(7, &cpu->hl.H, memory, cpu);
                    break;
                }
                case 0xFD:{ // SET 7, L. Set to 1 bit number 7.
                    set_u3_r8(7, &cpu->hl.L, memory, cpu);
                    break;
                }
                case 0xFE:{ // SET 7, [HL]. Set to 1 bit number 7 from address pointed by HL
                    set_u3_p16(7, &cpu->hl.HL, memory, cpu);
                    break;
                }
                case 0xFF:{ // SET 7, A. Set to 1 bit number 7.
                    set_u3_r8(7, &cpu->af.A, memory, cpu);
                    break;
                }

                default: {
                    printf("Invalid CB PREFIX instruction\n");
                }
            }
            break;
        }
        case 0xCC:{// CALL Z, a16. Save pc into stack and go to adress a16 if FLAG ZERO =1
            call_z_a16(memory, cpu);
            break;
        }
        case 0xCD:{// CALL a16. Save pc into stack and go to adress a16
            call_a16(memory, cpu);
            break;
        }
        case 0xCE:{// ADC A, n8. Sum A and n8 and carry flag.
            adc_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xCF:{// RST $08. Save current pc to stack and go to vector (0x08)
            rst_vec(0x08, memory, cpu);
            break;
        }
        case 0xD0:{// RET NC. Go to adress saved in sp (stack) if FLAG C = 0
            return_nc(memory, cpu);
            break;
        }
        case 0xD1:{// POP DE. Load the value in sp into DE
            pop_r16(&cpu->de.DE, memory, cpu);
            break;
        }
        case 0xD2:{// JP NC, a16 . Save current value in stack and jump to a16
            jump_pointer_a16(memory, cpu);
            break;
        }
        case 0xD3:{// NO INSTRUCTION.
            break;
        }
        case 0xD4:{// CALL NC, a16. Save current pc in stack and go to a16 if FLAG C = 0
            call_nc_a16(memory, cpu);
            break;
        }
        case 0xD5:{// PUSH DE. Save current pc into stack
            push_r16(&cpu->de.DE, memory, cpu);
            break;
        }
        case 0xD6:{// SUB A, n8. Substract n8 to A
            sub_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xD7:{// RST $10. Save current memory to stack and go to 0x10
            rst_vec(0x10, memory, cpu);
            break;
        }
        case 0xD8:{// RET C. Go to address saved in stack if CARRY = 1
            return_c(memory, cpu);
            break;
        }
        case 0xD9:{// RETI. Interruptions stuff
            cpu->pc = memory[cpu->sp] | (memory[cpu->sp + 1] << 8);
            cpu->sp += 2;
            cpu->ime = 1; // Enable interrupts
            cpu->cycles += 16;
            break;
        }
        case 0xDA:{// JP C, a16. Jump to a16 if FLAG C = 1
            jump_pointer_c_a16(memory, cpu);
            break;
        }
        case 0xDB:{//NO INSTRUCTION.
            break;
        }
        case 0xDC:{// CALL C, a16. Save pc in stack and go to a16 if FLAG C = 1
            call_c_a16(memory, cpu);
            break;
        }
        case 0xDD:{// NO INSTURCTION.
            break;
        }
        case 0xDE:{// SBC A, n8. Substract A and n8 and carry.
            sbc_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xDF:{// RST $18. Save current pc to stack and go to 0x18
            rst_vec(0x18, memory, cpu);
            break;
        }
        case 0xE0:{// LDH [a8], A. Copy value in A into a8
            load_a8_r8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xE1:{// POP HL. Save stack into HL
            pop_r16(&cpu->hl.HL, memory, cpu);
            break;
        }
        case 0xE2:{// LDH [C], A. Copy A into C
            load_c_r8(&cpu->bc.C, memory, cpu);
            break;
        }
        case 0xE3:{// NO INSTRUCTION
            break;
        }
        case 0xE4:{// NO INSTRUCTION
            break;
        }
        case 0xE5:{// PUSH HL. Save current pc into stack.
            push_r16(&cpu->hl.HL, memory, cpu);
            break;
        }
        case 0xE6:{// AND A, n8. Bitwise and between A and n8 parameter.
            and_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xE7:{// RST $20. Save current pc to stack and go to 0x20
            rst_vec(0x20, memory, cpu);
            break;
        }
        case 0xE8:{// ADD SP, e8. Sum signed 8 bit and sp (stack)
            add_sp_e8(memory, cpu);
            break;
        }
        case 0xE9:{// JP HL. Jump to address stored in HL.
            jump_pointer_HL(memory, cpu);
            break;
        }
        case 0xEA:{// LD [a16], A. Copy A into a16
            load_a16_r8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xEB:{// NO INSTRUCTION.
            break;
        }
        case 0xEC:{// NO INSTRUCTION.
            break;
        }
        case 0xED:{// NO INSTRUCTION.
            break;
        }
        case 0xEE:{// XOR A, n8. Bitwise XOR between A and n8 parameter.
            xor_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xEF:{// RST $28. Copy current sp to stack and go to 0x28
            rst_vec(0x28, memory, cpu);
            break;
        }
        case 0xF0:{// LDH A, [a8] .Copy a8 into A
            load_r8_a8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xF1:{// POP AF. Load sp (stack) into AF
            pop_r16(&cpu->af.AF, memory, cpu);
            break;
        }
        case 0xF2:{// LDH A, [C]. Load relative C into A
            load_r8_c(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xF3:{// DI. Interruptions stuff
            cpu->ime = false;
            cpu->cycles += 4;
            break;
        }
        case 0xF4:{// NO INSTRUCTION
            break;
        }
        case 0xF5:{// PUSH AF. Copy current pc int ostack.
            push_r16(&cpu->af.AF, memory, cpu);
            break;
        }
        case 0xF6:{// OR A, n8. Bitwise OR between A and n8 parameter
            or_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xF7:{// RST $30. Save current pc to stack and go to 0x30
            rst_vec(0x30, memory, cpu);
            break;
        }
        case 0xF8:{// LD HL, SP + e8. Add SP to e8 and copy it to HL
            load_hl_sp_e8(memory, cpu);
            break;
        }
        case 0xF9:{// LD SP, HL. Copy HL into SP
            load_sp_hl(memory, cpu);
            break;
        }
        case 0xFA:{// LD A, [a16]. Copy HL into SP
            load_r8_a16(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xFB:{// EI. Interruptions stuff.
            cpu->ime = true;
            cpu->cycles += 4;
            break;
        }
        case 0xFC:{// NO INSTRUCTION
            break;
        }
        case 0xFD:{// NO INSTRUCTION
            break;
        }
        case 0xFE:{// CP A, n8.
            cp_r8_n8(&cpu->af.A, memory, cpu);
            break;
        }
        case 0xFF:{// RST $38. Save current pc to stack and go to 0x38
            rst_vec(0x38, memory, cpu);
            break;
        }
        default:
            printf("Opcode 0x%02X not implemented\n", opcode);
            break;
    }

    handle_interrupts(memory, cpu);
    return cpu->cycles;
}
