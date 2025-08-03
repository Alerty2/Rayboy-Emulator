#include "ppu.h"
#include "mmu.h"
#include <stdlib.h>

void ppu_init(PPU* ppu, MMU_ADDRESSES* mmu_addresses){
    ppu->mode = 2;
    ppu->scanline = 0;
    ppu->cycle = 0;
    ppu->windowLineCounter = 0;
    memset(ppu->framebuffer, 0, sizeof(ppu->framebuffer));
    write_byte(mmu_addresses->LCDC, 0x91);
    write_byte(mmu_addresses->LY, 144);
    write_byte(mmu_addresses->STAT, 0x80);
}
void ppu_step(PPU* ppu, int cycles, MMU_ADDRESSES* mmu_addresses) {
    ppu->cycle += cycles;

    switch (ppu->mode) {
        case 2: // OAM
            if (ppu->cycle >= 80) {
                ppu->cycle -= 80;
                ppu->mode = 3;
                set_ppu_mode(mmu_addresses, 3);
                set_ppu_mode(mmu_addresses, 3);
            }
            break;

        case 3: // Drawing
            if (ppu->cycle >= 172) {
                ppu->cycle -= 172;
                // Render scanline antes de cambiar de modo
                if (ppu->scanline < 144) {
                    ppu_render_scanline(ppu, mmu_addresses);
                }

                ppu->mode = 0;
                set_ppu_mode(mmu_addresses, 0);

                uint8_t stat = read_byte(mmu_addresses->STAT);
                if (stat & 0x08) {
                    uint8_t iflag = read_byte(mmu_addresses->IF);
                    write_byte(mmu_addresses->IF, iflag | 0x02);
                }
            }
            break;

        case 0: // HBlank
            // Mode 0 (HBlank)
            if (ppu->cycle >= (456 - 80 - 172)) {
                ppu->cycle -= (456 - 80 - 172);
                ppu->scanline++;
                write_byte(mmu_addresses->LY, ppu->scanline);
                set_lyc_flag(ppu, mmu_addresses);
                if (ppu->scanline == 144) {
                    ppu->mode = 1;
                    set_ppu_mode(mmu_addresses, 1);

                    uint8_t iflag = read_byte(mmu_addresses->IF);
                    write_byte(mmu_addresses->IF, iflag | 0x01); // VBlank

                    uint8_t stat = read_byte(mmu_addresses->STAT);
                    if (stat & 0x10) {
                        write_byte(mmu_addresses->IF, iflag | 0x02); // STAT interrupt
                    }

                } else {
                    ppu->mode = 2;
                    set_ppu_mode(mmu_addresses, 2);

                    uint8_t stat = read_byte(mmu_addresses->STAT);
                    if (stat & 0x20) {
                        uint8_t iflag = read_byte(mmu_addresses->IF);
                        write_byte(mmu_addresses->IF, iflag | 0x02);
                    }
                }
            }
            break;

        case 1: // VBlank
            if (ppu->cycle >= 456) {
                ppu->cycle -= 456;
                ppu->scanline++;

                if (ppu->scanline >= 153) {
                    ppu->scanline = 0;
                    ppu->mode = 2;
                    set_ppu_mode(mmu_addresses, 2);
                }

                write_byte(mmu_addresses->LY, ppu->scanline);
                set_lyc_flag(ppu, mmu_addresses);
            }
            break;
    }
}
void test_frame(PPU* ppu) {
    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            ppu->framebuffer[y][x] = (x / 8 + y / 8) % 4;
        }
    }
}
Color get_palette_color(uint8_t color){
    switch (color){
        case 0: return WHITE;
        case 1: return LIGHTGRAY;
        case 2: return DARKGRAY;
        case 3: return BLACK;
        default: return RED; // ERROR
    }
}
void display_frame(PPU* ppu){
    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            uint8_t color = ppu->framebuffer[y][x];
            Color real_color = get_palette_color(color);
            DrawPixel(x, y, real_color);
        }
    }
}
void ppu_render_scanline(PPU* ppu, MMU_ADDRESSES* mmu_addresses) {
    int y = read_byte(mmu_addresses->LY);
    uint8_t lcdc  = read_byte(mmu_addresses->LCDC);
    uint8_t scx   = read_byte(mmu_addresses->SCX);
    uint8_t scy   = read_byte(mmu_addresses->SCY);
    uint8_t bgp   = read_byte(mmu_addresses->BGP);
    uint8_t ly    = read_byte(mmu_addresses->LY);
    uint8_t wx    = read_byte(mmu_addresses->WX);
    uint8_t wy    = read_byte(mmu_addresses->WY);
    uint8_t obp1  = read_byte(mmu_addresses->OBP1);
    uint8_t obp0  = read_byte(mmu_addresses->OBP0);

    // Render Background
    render_background(ppu, mmu_addresses);
    // Render Window
    render_window(ppu, mmu_addresses);
    // Render Sprites
    render_sprites(ppu, mmu_addresses);
}

void set_lyc_flag(PPU* ppu, MMU_ADDRESSES* mmu_addresses){

    if (ppu->scanline == read_byte(mmu_addresses->LYC)){
        uint8_t stat = read_byte(mmu_addresses->STAT);
        stat |= 0x04;
        write_byte(mmu_addresses->STAT, stat);

        if ((read_byte(mmu_addresses->STAT) & 0x40) != 0){
            write_byte(mmu_addresses->IF, (uint8_t)(mmu_addresses->IF | 0x02));
        }else{
            stat &= 0xBF;
            write_byte(mmu_addresses->STAT, stat);
        }
    }
}
void set_ppu_mode(MMU_ADDRESSES* mmu, uint8_t mode) {
    uint8_t stat = read_byte(mmu->STAT);
    stat &= 0xFC; // Limpia bits de modo
    stat |= (mode & 0x03); // Establece nuevo modo
    write_byte(mmu->STAT, stat);
}
void debug_render(PPU* ppu) {
    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            // Patrón de prueba más visible (franjas verticales)
            ppu->framebuffer[y][x] = (x / 10) % 4;
        }
    }
}

void render_background(PPU* ppu, MMU_ADDRESSES* mmu_addresses){

}
void render_window(PPU* ppu, MMU_ADDRESSES* mmu_addresses){

}
void render_sprites(PPU* ppu, MMU_ADDRESSES* mmu_addresses){

}

void display_vram(PPU* ppu, MMU_ADDRESSES* mmu_addresses, int* x, int* y, int* dev){
    int ky = 10;
    int oy = 0;
    int kx = 10;
    int ox = 10;
    int line = 145;
    *x = ox;
    *y = 0;

    for (int i = 0x8000; i < 0x97FF; i++){
        *x += kx;
        if (*x >= ((line * kx) + ox)){
            *y += ky;
            *x = ox;
        }
        DrawText(TextFormat("%02X", read_byte(i)), *x,*y += *dev, 10, BLACK);
    }
}
