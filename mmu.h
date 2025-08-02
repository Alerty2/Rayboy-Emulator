#ifndef MMU_H
#define MMU_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define EXT_RAM_BANKS   4
#define EXT_RAM_SIZE    (EXT_RAM_BANKS * 0x2000)

extern bool    ram_enable;
extern uint8_t current_ram_bank;
extern uint8_t mbc1_mode;

// Memory Buffers
extern uint8_t vram[0x2000];
extern uint8_t wram[0x2000];
extern uint8_t ext_ram[EXT_RAM_SIZE];
extern uint8_t io_and_high[0x0200];

#define ROM_BANK_SIZE 0x4000
#define MAX_ROM_BANKS 128
extern uint8_t* rom_banks[MAX_ROM_BANKS];
extern int     rom_bank_count;
extern int     current_rom_bank;

int  load_rom_banks(const char* path);

uint8_t read_byte(uint16_t addr);
void write_byte(uint16_t addr, uint8_t val);
void init_mmu();
void printVRAM(uint8_t *vRAM);

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
#endif // MMU_H
