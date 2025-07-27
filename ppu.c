#include "ppu.h"

void ppu_init(PPU* ppu){
    ppu->mode = 2;
    ppu->scanline = 0;
    ppu->cycle = 0;
    memset(ppu->framebuffer, 0, sizeof(ppu->framebuffer));
}
void ppu_step(PPU* ppu, int cycles, uint8_t* memory, MMU_ADDRESSES* mmu_addresses) {
    ppu->cycle += cycles;


    switch (ppu->mode) {
        case 2: // OAM
            if (ppu->cycle >= 80) {
                ppu->cycle -= 80;
                ppu->mode = 3;

                set_ppu_mode(memory, mmu_addresses, 2);
            }
            break;

        case 3: // Drawing
            if (ppu->cycle >= 172) {
                ppu->cycle -= 172;
                ppu->mode = 0;
                set_ppu_mode(memory, mmu_addresses, 2);
                if (ppu->scanline < 144){
                    ppu_render_scanline(ppu, memory); // Draw actual line
                }
                if ((mmu_addresses->STAT & 0x08) != 0){
                    mmu_write(memory, mmu_addresses->IF, (uint8_t)(mmu_addresses->IF | 0x02));
                }
            }
            break;

        case 0: // HBlank
            if (ppu->cycle >= (456 - 80 - 172)) {
                ppu->cycle -= (456 - 80 - 172);
                ppu->scanline++;

                set_lyc_flag(memory, ppu, mmu_addresses);

                if (ppu->scanline == 144) {
                    ppu->mode = 1;
                    memory[0xFF0F] |= 0x01; // Throw VBlank Interruption
                    //display_frame(ppu);
                    if ((mmu_load(memory, mmu_addresses->STAT) & 0x10) != 0){
                        mmu_write(memory, mmu_addresses->IF, (uint8_t)(mmu_addresses->IF | 0x02));
                    }
                } else {
                    if ((mmu_load(memory, mmu_addresses->STAT) & 0x20) != 0){
                        mmu_write(memory, mmu_addresses->IF, (uint8_t)(mmu_addresses->IF | 0x02));
                    }
                    ppu->mode = 2;
                }
                memory[0xFF44] = ppu->scanline; // LY
                set_ppu_mode(memory, mmu_addresses, 2);
            }
            break;

        case 1: // VBlank
            if (ppu->scanline == 144){
                if((mmu_load(memory, mmu_addresses->LCDC) & 80) != 0){
                    mmu_write(memory, mmu_addresses->IF, (uint8_t)(mmu_addresses->IF | 0x01));
                }
            }
            if (ppu->cycle >= 456) {

                ppu->cycle -= 456;
                ppu->scanline++;
                set_lyc_flag(memory, ppu, mmu_addresses);
                if (ppu->scanline == 153) {
                    ppu->scanline = 0;
                    ppu->mode = 2;
                    memory[0xFF44] = 0;
                    set_ppu_mode(memory, mmu_addresses, 2);
                    if ((mmu_load(memory, mmu_addresses->STAT) & 0x20) != 0){
                        mmu_write(memory, mmu_addresses->IF, (uint8_t)(mmu_addresses->IF | 0x02));
                    }
                } else {
                    memory[0xFF44] = ppu->scanline;
                }
            }
            break;
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
void ppu_render_scanline(PPU* ppu, uint8_t* memory) {
    uint8_t lcdc = memory[0xFF40];
    if (!(lcdc & 0x80)) return; // LCD OFF

    int y = ppu->scanline;
    uint8_t scy = memory[0xFF42]; // Scroll Y
    uint8_t scx = memory[0xFF43]; // Scroll X
    for (int x = 0; x < 160; x++) ppu->framebuffer[y][x] = (x + y) % 4;

    // BG tile map area
    uint16_t bg_tile_map_addr = (lcdc & 0x08) ? 0x9C00 : 0x9800;

    // Tile data select
    int signed_index = !(lcdc & 0x10); // 0 = signed indices
    uint16_t tile_data_base = signed_index ? 0x8800 : 0x8000;

    for (int x = 0; x < 160; x++) {
        // Calculate position in tilemap
        uint8_t scroll_x = x + scx;
        uint8_t scroll_y = y + scy;

        uint16_t tile_x = scroll_x / 8;
        uint16_t tile_y = scroll_y / 8;

        uint16_t tile_index_addr = bg_tile_map_addr + tile_y * 32 + tile_x;
        int8_t tile_index = memory[tile_index_addr]; // Can be signed if mode 0x8800

        uint16_t tile_addr;
        if (signed_index)
            tile_addr = 0x9000 + ((int8_t)tile_index) * 16;
        else
            tile_addr = tile_data_base + tile_index * 16;

        // Line in tile
        uint8_t tile_row = (scroll_y % 8) * 2;
        uint8_t byte1 = memory[tile_addr + tile_row];
        uint8_t byte2 = memory[tile_addr + tile_row + 1];

        int bit = 7 - (scroll_x % 8);
        uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

        // Map the palet with BGP
        uint8_t palette = memory[0xFF47]; // BGP
        uint8_t color = (palette >> (color_id * 2)) & 0x03;

        ppu->framebuffer[y][x] = color;
    }
}

void set_lyc_flag(uint8_t memory[], PPU* ppu, MMU_ADDRESSES* mmu_addresses){

    if (ppu->scanline == mmu_load(memory, mmu_addresses->LYC)){
        uint8_t stat = mmu_load(memory, mmu_addresses->STAT);
        stat |= 0x04;
        mmu_write(memory, mmu_addresses->STAT, stat);

        if ((mmu_load(memory, mmu_addresses->STAT) & 0x40) != 0){
            mmu_write(memory, mmu_addresses->IF, (uint8_t)(mmu_addresses->IF | 0x02));
        }else{
            stat &= 0xBF;
            mmu_write(memory, mmu_addresses->STAT, stat);
        }
    }
}
void set_ppu_mode(uint8_t* memory, MMU_ADDRESSES* mmu, uint8_t mode) {
    uint8_t stat = mmu_load(memory, mmu->STAT);
    stat = (stat & 0xFC) | (mode & 0x03);
    mmu_write(memory, mmu->STAT, stat);
}
