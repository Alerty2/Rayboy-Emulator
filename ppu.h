#ifndef PPU_H
#define PPU_H
#include <stdint.h>
#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include "mmu.h"

typedef struct{
    int mode; // 0: HBlank, 1: VBlank, 2: OAM, 3: Pixel transfer
    int scanline; // LY (0–153)
    int cycle; // Cycle counter (0-456 per scanline)
    uint8_t framebuffer[144][160];
    int windowLineCounter;
} PPU;

void ppu_init(PPU* ppu, MMU_ADDRESSES* mmu_addresses);
void ppu_step(PPU* ppu, int cycles, MMU_ADDRESSES* mmu_addresses);
void ppu_render_scanline(PPU* ppu, MMU_ADDRESSES* mmu_addresses);
void display_frame(PPU* ppu);
Color get_palette_color(uint8_t color);
void set_ppu_mode(MMU_ADDRESSES* mmu, uint8_t mode);
void test_frame(PPU* ppu);
void debug_render(PPU* ppu);

void render_background(PPU* ppu, MMU_ADDRESSES* mmu_addresses);
void render_window(PPU* ppu, MMU_ADDRESSES* mmu_addresses);
void render_sprites(PPU* ppu, MMU_ADDRESSES* mmu_addresses);

void display_vram(PPU* ppu, MMU_ADDRESSES* mmu_addresses, int* x, int* y, int* dev);

// PPU flags
void set_lyc_flag(PPU* ppu, MMU_ADDRESSES* mmu_addresses);
#endif
