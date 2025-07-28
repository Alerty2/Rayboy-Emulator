#include "mmu.h"

typedef struct{
    int mode; // 0: HBlank, 1: VBlank, 2: OAM, 3: Pixel transfer
    int scanline; // LY (0–153)
    int cycle; // Cycle counter (0-456 per scanline)
    uint8_t framebuffer[144][160];
    int windowLineCounter;
} PPU;

void ppu_init(PPU* ppu);
void ppu_step(PPU* ppu, int cycles, uint8_t* memory, MMU_ADDRESSES* mmu_addresses);
void ppu_render_scanline(PPU* ppu, uint8_t* memory, MMU_ADDRESSES* mmu_addresses);
void display_frame(PPU* ppu);
Color get_palette_color(uint8_t color);
void set_ppu_mode(uint8_t* memory, MMU_ADDRESSES* mmu, uint8_t mode);
void test_frame(PPU* ppu);

// PPU flags
void set_lyc_flag(uint8_t memory[], PPU* ppu, MMU_ADDRESSES* mmu_addresses);
