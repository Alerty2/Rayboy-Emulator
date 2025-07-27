#include "emulation.h"

void mmu_write(uint8_t memory[], uint16_t address, uint8_t value);
uint8_t mmu_load(uint8_t memory[], uint16_t address);

// Addresses
typedef struct{
    uint16_t IE;
    uint16_t IF;
    uint16_t JOYP;
    uint16_t DIV;
    uint16_t TIMA;
    uint16_t TMA;
    uint16_t TAC;
    uint16_t LCDC;
    uint16_t STAT;
    uint16_t SCY;
    uint16_t SCX;
    uint16_t LY;
    uint16_t LYC;
    uint16_t BGP;
    uint16_t OBP0;
    uint16_t OBP1;
    uint16_t WY;
    uint16_t WX;
} MMU_ADDRESSES;

void init_mmu_addresses(MMU_ADDRESSES* mmu_addresses);
