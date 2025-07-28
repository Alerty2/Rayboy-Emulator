#include "ppu.h"

void ppu_init(PPU* ppu){
    ppu->mode = 2;
    ppu->scanline = 0;
    ppu->cycle = 0;
    ppu->windowLineCounter = 0;
    memset(ppu->framebuffer, 0, sizeof(ppu->framebuffer));
}
void ppu_step(PPU* ppu, int cycles, uint8_t* memory, MMU_ADDRESSES* mmu_addresses) {
    ppu->cycle += cycles;

    switch (ppu->mode) {
        case 2: // OAM
            if (ppu->cycle >= 80) {
                ppu->cycle -= 80;
                ppu->mode = 3;
                set_ppu_mode(memory, mmu_addresses, 3);
            }
            break;

        case 3: // Drawing
            if (ppu->cycle >= 172) {
                ppu->cycle -= 172;
                // Render scanline antes de cambiar de modo
                if (ppu->scanline < 144) {
                    ppu_render_scanline(ppu, memory, mmu_addresses);
                }

                ppu->mode = 0;
                set_ppu_mode(memory, mmu_addresses, 0);

                uint8_t stat = mmu_load(memory, mmu_addresses->STAT);
                if (stat & 0x08) {
                    uint8_t iflag = mmu_load(memory, mmu_addresses->IF);
                    mmu_write(memory, mmu_addresses->IF, iflag | 0x02);
                }
            }
            break;

        case 0: // HBlank
            if (ppu->cycle >= (456 - 80 - 172)) {
                ppu->cycle -= (456 - 80 - 172);
                ppu->scanline++;

                memory[0xFF44] = ppu->scanline; // LY
                set_lyc_flag(memory, ppu, mmu_addresses);

                if (ppu->scanline == 144) {
                    ppu->mode = 1;
                    set_ppu_mode(memory, mmu_addresses, 1);

                    uint8_t iflag = mmu_load(memory, mmu_addresses->IF);
                    mmu_write(memory, mmu_addresses->IF, iflag | 0x01); // VBlank

                    uint8_t stat = mmu_load(memory, mmu_addresses->STAT);
                    if (stat & 0x10) {
                        mmu_write(memory, mmu_addresses->IF, iflag | 0x02); // STAT interrupt
                    }

                    // Podés dibujar el frame aquí si usás SDL
                    // display_frame(ppu);
                } else {
                    ppu->mode = 2;
                    set_ppu_mode(memory, mmu_addresses, 2);

                    uint8_t stat = mmu_load(memory, mmu_addresses->STAT);
                    if (stat & 0x20) {
                        uint8_t iflag = mmu_load(memory, mmu_addresses->IF);
                        mmu_write(memory, mmu_addresses->IF, iflag | 0x02);
                    }
                }
            }
            break;

        case 1: // VBlank
            if (ppu->cycle >= 456) {
                ppu->cycle -= 456;
                ppu->scanline++;

                if (ppu->scanline > 153) {
                    ppu->scanline = 0;
                    ppu->mode = 2;
                    set_ppu_mode(memory, mmu_addresses, 2);
                }

                memory[0xFF44] = ppu->scanline;
                set_lyc_flag(memory, ppu, mmu_addresses);
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
void ppu_render_scanline(PPU* ppu, uint8_t* memory, MMU_ADDRESSES* mmu_addresses) {
    int y = mmu_load(memory, mmu_addresses->LY);
    printf("Render scanline: LY = %d\n", y);
    uint8_t lcdc = mmu_load(memory, mmu_addresses->LCDC);
    uint8_t scx = mmu_load(memory, mmu_addresses->SCX);
    uint8_t scy = mmu_load(memory, mmu_addresses->SCY);
    uint8_t bgp = mmu_load(memory, mmu_addresses->BGP);
    uint8_t ly = mmu_load(memory, mmu_addresses->LY);
    uint8_t wx = mmu_load(memory, mmu_addresses->WX);
    uint8_t wy = mmu_load(memory, mmu_addresses->WY);
    uint8_t obp1 = mmu_load(memory, mmu_addresses->OBP1);
    uint8_t obp0 = mmu_load(memory, mmu_addresses->OBP0);

    // Render Background
    if ((lcdc & 0x01) == 0) return;  // Background display off

    for (int x = 0; x < 160; x++) {
        int bgX = (scx + x) % 256;
        int bgY = (scy + y) % 256;

        int tileX = bgX / 8;
        int tileY = bgY / 8;
        int tileIndex = tileY * 32 + tileX;

        uint16_t tileMapBase = (lcdc & 0x08) ? 0x9C00 : 0x9800;
        uint8_t tileNumber = mmu_load(memory, tileMapBase + tileIndex);

        uint16_t tileDataBase;
        if (lcdc & 0x10) {
            tileDataBase = 0x8000;
        } else {
            tileDataBase = 0x9000 + (int8_t)tileNumber * 16;
        }

        uint8_t lineInTile = bgY % 8;
        uint16_t tileAddress = tileDataBase + lineInTile * 2;

        uint8_t byte1 = mmu_load(memory, tileAddress);
        uint8_t byte2 = mmu_load(memory, tileAddress + 1);

        int bit = 7 - (bgX % 8);
        uint8_t colorId = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

        uint8_t paletteColor = (bgp >> (colorId * 2)) & 0x03;

        ppu->framebuffer[y][x] = paletteColor;
    }
    // Render Window
    if ((lcdc & (1 << 5)) == 0) return;

    int currentScanline = ly;
    int windowX = wx - 7;
    int windowY = wy;

    if (currentScanline < windowY) return;

    if (currentScanline == windowY) {
        ppu->windowLineCounter = 0;
    }

    ushort tileMapBase = (lcdc & (1 << 6)) != 0 ? (ushort)0x9C00 : (ushort)0x9800;

    bool windowRendered = false;

    for (int x = 0; x < 160; x++)
    {
        if (x < windowX) continue;

        windowRendered = true;

        int windowColumn = x - windowX;

        int tileX = windowColumn / 8;
        int tileY = ppu->windowLineCounter / 8;

        int tileIndex = tileY * 32 + tileX;

        uint8_t tileNumber = mmu_load(memory, tileMapBase + tileIndex);

        ushort tileDataBase = (lcdc & (1 << 4)) != 0 || tileNumber >= 128 ? (ushort)0x8000 : (ushort)0x9000;
        ushort tileAddress = (ushort)(tileDataBase + tileNumber * 16);

        int lineInTile = ppu->windowLineCounter % 8;

        uint8_t tileLow = mmu_load(memory, tileAddress + lineInTile * 2);
        uint8_t tileHigh = mmu_load(memory, tileAddress + lineInTile * 2 + 1);

        int bitIndex = 7 - (windowColumn % 8);
        int colorBit = ((tileHigh >> bitIndex) & 1) << 1 | ((tileLow >> bitIndex) & 1);

        int paletteShift = colorBit * 2;
        int paletteColor = (bgp >> paletteShift) & 0b11;
        //return ConvertPaletteColor(paletteColor);
        if (y == 72 && x == 80) {
            printf("TileMapBase: %04X, tileIndex: %d, tileNumber: %02X, tileAddr: %04X\n",
                   tileMapBase, tileIndex, tileNumber, tileAddress);
        }

        ppu->framebuffer[y][x] = paletteColor;
    }

    if (windowRendered) {
        ppu->windowLineCounter++;
    }

    // Render Sprites
    if ((lcdc & (1 << 1)) == 0) return;  // OBJ display disabled

    int spriteHeight = (lcdc & (1 << 2)) ? 16 : 8;
    int renderedSprites = 0;
    int pixelOwner[160];
    for (int i = 0; i < 160; i++) pixelOwner[i] = -1;

    for (int i = 0; i < 40; i++) {
        if (renderedSprites >= 10) break;

        int spriteIndex = i * 4;
        int yPos = mmu_load(memory, 0xFE00 + spriteIndex) - 16;
        int xPos = mmu_load(memory, 0xFE00 + spriteIndex + 1) - 8;
        uint8_t tileIndex = mmu_load(memory, 0xFE00 + spriteIndex + 2);
        uint8_t attributes = mmu_load(memory, 0xFE00 + spriteIndex + 3);

        if (y < yPos || y >= yPos + spriteHeight) continue;

        int lineInSprite = y - yPos;
        if (attributes & (1 << 6)) {
            lineInSprite = spriteHeight - 1 - lineInSprite;
        }

        if (spriteHeight == 16) {
            tileIndex &= 0xFE;
            if (lineInSprite >= 8) {
                tileIndex += 1;
                lineInSprite -= 8;
            }
        }

        uint16_t tileAddress = 0x8000 + tileIndex * 16 + lineInSprite * 2;
        uint8_t tileLow = mmu_load(memory, tileAddress);
        uint8_t tileHigh = mmu_load(memory, tileAddress + 1);

        for (int x = 0; x < 8; x++) {
            int bitIndex = (attributes & (1 << 5)) ? x : 7 - x;
            int colorBit = ((tileHigh >> bitIndex) & 1) << 1 | ((tileLow >> bitIndex) & 1);
            if (colorBit == 0) continue;

            int screenX = xPos + x;
            if (screenX < 0 || screenX >= 160) continue;

            // BG-to-OAM priority
            if ((attributes & (1 << 7)) &&
                ppu->framebuffer[y][screenX] != 0) {
                continue;
            }

            if (pixelOwner[screenX] == -1 || xPos < pixelOwner[screenX]) {
                pixelOwner[screenX] = xPos;
                bool isSpritePalette1 = attributes & (1 << 4);
                uint8_t spritePalette = isSpritePalette1 ? obp1 : obp0;

                int paletteShift = colorBit * 2;
                int paletteColor = (spritePalette >> paletteShift) & 0x03;

                ppu->framebuffer[y][screenX] = paletteColor;
            }
        }

        renderedSprites++;
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
