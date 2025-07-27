#include "mmu.h"

void mmu_write(uint8_t memory[], uint16_t address, uint8_t value){
    memory[address] = value;
}
uint8_t mmu_load(uint8_t memory[], uint16_t address){
    return memory[address];
}

void init_mmu_addresses(MMU_ADDRESSES* mmu_addresses){
    mmu_addresses->IE = 0xFFFF;
    mmu_addresses->IF = 0xFF0F;
    mmu_addresses->JOYP = 0xFF00;
    mmu_addresses->DIV = 0xFF04;
    mmu_addresses->TIMA = 0xFF05;
    mmu_addresses->TMA = 0xFF06;
    mmu_addresses->TAC = 0xFF07;
    mmu_addresses->LCDC = 0xFF40;
    mmu_addresses->STAT = 0xFF41;
    mmu_addresses->SCY = 0xFF42;
    mmu_addresses->SCX = 0xFF43;
    mmu_addresses->LY = 0xFF44;
    mmu_addresses->LYC = 0xFF54;
    mmu_addresses->BGP = 0xFF47;
    mmu_addresses->OBP0 = 0xFF48;
    mmu_addresses->OBP1 = 0xFF49;
    mmu_addresses->WY = 0xFF4A;
    mmu_addresses->WX = 0xFF4B;
}
