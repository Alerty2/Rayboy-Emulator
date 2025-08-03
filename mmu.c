#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROM_BANK_SIZE 0x4000
#define MAX_ROM_BANKS 128


bool    ram_enable = false;
uint8_t current_ram_bank = 0;
uint8_t mbc1_mode = 0;

uint8_t vram[0x2000];
uint8_t wram[0x2000];
uint8_t ext_ram[EXT_RAM_SIZE];       // ajusta según RAM de cartucho
uint8_t io_and_high[0x0200];

uint8_t* rom_banks[MAX_ROM_BANKS];
int      rom_bank_count = 0;
int      current_rom_bank = 1;
int counter = 0;
void init_mmu() {
    memset(vram, 0, sizeof(vram));
    memset(wram, 0, sizeof(wram));
    memset(ext_ram, 0, sizeof(ext_ram));
    memset(io_and_high, 0, sizeof(io_and_high));
    // Reset bancos
    current_rom_bank = 1;
    current_ram_bank = 0;
    ram_enable = false;
    mbc1_mode = 0;

}

int load_rom_banks(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return -1;
    // calcula tamaño total
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    rom_bank_count = (size + ROM_BANK_SIZE - 1) / ROM_BANK_SIZE;
    if (rom_bank_count > MAX_ROM_BANKS) rom_bank_count = MAX_ROM_BANKS;
    for (int i = 0; i < rom_bank_count; i++) {
        rom_banks[i] = malloc(ROM_BANK_SIZE);
        size_t read = fread(rom_banks[i], 1, ROM_BANK_SIZE, f);
        if (read < ROM_BANK_SIZE) {
            // rellena con 0xFF si queda vacío
            memset(rom_banks[i] + read, 0xFF, ROM_BANK_SIZE - read);
        }
    }
    fclose(f);
    return 0;
}

uint8_t read_byte(uint16_t addr) {
    if (addr < 0x4000) {
        return rom_banks[0][addr];
    }
    else if (addr < 0x8000) {
        int bank = current_rom_bank % rom_bank_count;
        if (bank == 0) bank = 1;  // Banco 0 no es válido aquí
        return rom_banks[bank][addr - 0x4000];
    }
    else if (addr < 0xA000) {
        //printf("Reading Byte: %04X from VRAM\n", addr);
        return vram[addr - 0x8000];
    }
    else if (addr < 0xC000) {
        //printf("Reading Byte: %04X from cartidge\n", addr);
        return ext_ram[addr - 0xA000 + current_ram_bank * 0x2000];
    }
    else if (addr < 0xE000) {
        //printf("Reading Byte: %04X from wRAM\n", addr);
        return wram[addr - 0xC000];
    }
    else if (addr < 0xFE00) {
        //printf("Reading Byte: %04X from mirrored RAM\n", addr);

        return wram[addr - 0xE000];
    }
    else {
        //printf("Reading Byte: %04X from IO and higher registers\n", addr);
        return io_and_high[addr - 0xFE00];
    }
}

void write_byte(uint16_t addr, uint8_t val) {
    if (addr < 0x2000) {
        ram_enable = (val & 0x0F) == 0x0A;
        printf("Writing Byte: %04X to MBC1 (RAM enable: %d)\n", addr, ram_enable);
    }
    else if (addr < 0x4000) {
        uint8_t low = val & 0x1F;
        current_rom_bank = (current_rom_bank & 0x60) | (low ? low : 1);
        printf("Writing Byte: %04X to MBC1 (ROM bank low: %d)\n", addr, low);
    }
    else if (addr < 0x6000) {
        if (mbc1_mode == 0) {
            current_rom_bank = (current_rom_bank & 0x1F) | ((val & 0x03) << 5);
            printf("Writing Byte: %04X to MBC1 (ROM bank high)\n", addr);
        } else {
            current_ram_bank = val & 0x03;
            printf("Writing Byte: %04X to MBC1 (RAM bank: %d)\n", addr, current_ram_bank);
        }
    }
    else if (addr < 0x8000) {
        mbc1_mode = val & 0x01;
        printf("Writing Byte: %04X to MBC1 (Mode: %d)\n", addr, mbc1_mode);
    }
    else if (addr < 0xA000) {
        if (addr >= 0x8000 && addr <= 0x87FF){
            printf("Writing Byte: %04X to VRAM Block 0 (tiles)\n", addr);
            vram[addr - 0x8000] = val;
        }else if (addr >= 0x8800 && addr <= 0x8FFF){
            printf("Writing Byte: %04X to VRAM Block 1 (tiles)\n", addr);
            vram[addr - 0x8000] = val;
        }else if (addr >= 0x9000 && addr <= 0x97FF){
            printf("Writing Byte: %04X to VRAM Block 2 (tiles)\n", addr);
            vram[addr - 0x8000] = val;
        }else if(addr >= 0x9800 && addr <= 0x9BFF){
            printf("Writing Byte: %04X to VRAM Tile Map\n", addr);
            vram[addr - 0x8000] = val;
        }
        else{
            printf("Writing Byte: %04X to VRAM 1\n", addr);
            vram[addr - 0x8000] = val;
        }

    }
    else if (addr < 0xC000) {
        printf("Writing Byte: %04X to VRAM 2\n", addr);
        if (ram_enable) {
            ext_ram[addr - 0xA000 + current_ram_bank * 0x2000] = val;
        }
    }
    else if (addr < 0xE000) {
        printf("Writing Byte: %04X to 7\n", addr);
        wram[addr - 0xC000] = val;
    }
    else if (addr < 0xFE00) {
        printf("Writing Byte: %04X to 8\n", addr);
        // espejo WRAM
        wram[addr - 0xE000] = val;
    }
    else {
        if (addr >= 0xFE00 && addr <= 0xFE9F){
            printf("Writing Byte: %04X to OAM\n", addr);
            io_and_high[addr - 0xFE00] = val;
        }
        else{
            //printf("Writing Byte: %04X to 9\n", addr);
            io_and_high[addr - 0xFE00] = val;
        }
    }
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
    mmu_addresses->LYC = 0xFF45;
    mmu_addresses->BGP = 0xFF47;
    mmu_addresses->OBP0 = 0xFF48;
    mmu_addresses->OBP1 = 0xFF49;
    mmu_addresses->WY = 0xFF4A;
    mmu_addresses->WX = 0xFF4B;
}

void printVRAM(uint8_t *vRAM){
    for (int i = 0; i < 0x2000; i++){
        printf("%02X", vRAM[i]);
    }
}
