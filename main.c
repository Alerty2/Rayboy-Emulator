#include "loader.h"
#include "emulation.h"
#include "ppu.h"
#include "input.h"
#include <raylib.h>

// Define the RAM
//uint8_t wRAM[8192];

// Define the memory
uint8_t memory[0x10000];

// Define CPU
CPU cpu;
// Define PPU
PPU ppu;
// Define MMU addresses
MMU_ADDRESSES mmu_addresses;

// Input state
uint8_t input_state;

int y = 0;
int x = 0;
int dev = 0;
int main(int argc, char* argv[])
{
    // Check if ROM was provided
    if (argc < 2) {
        printf("Usage: %s <rom_file.gb>\n", argv[0]);
        return 1;
    }

    // Load ROM from provided path
    if (load_rom_banks(argv[1]) < 0) {
        printf("Failed to load ROM banks: %s\n", argv[1]);
        return 1;
    }
    printf("\n");
    // Después de load_rom_banks(...)
    //memset(vram,        0, sizeof(vram));         // 0x8000–0x9FFF
    //memset(ext_ram,     0, sizeof(ext_ram));      // 0xA000–0xBFFF (si la usas)
    memset(wram,        0, sizeof(wram));         // 0xC000–0xDFFF
    memset(io_and_high, 0, sizeof(io_and_high));  // 0xE000–0xFFFF

    write_byte(0xFF40, 0x91);   // LCDC: LCD ON + BG ON + Tile map 0
    write_byte(0xFF42, 0x00);   // SCY
    write_byte(0xFF43, 0x00);   // SCX
    write_byte(0xFF44, 0x00);   // LY = 0
    write_byte(0xFF47, 0xE4);   // BGP: paleta White->Black

    printf("LCDC after init = %02X\n", read_byte(0xFF40));
    printf("LY   after init = %02X\n", read_byte(0xFF44));
    printf("Tile data base: %02X %02X %02X %02X\n",
            read_byte(0x8000), read_byte(0x8001),
            read_byte(0x8002), read_byte(0x8003));

    // Initialize CPU state
    cpu.pc = 0x100;
    cpu.sp = 0xFFFE;
    cpu.af.AF = 0x01B0;
    cpu.bc.BC = 0x0013;
    cpu.de.DE = 0x00D8;
    cpu.hl.HL = 0x014D;
    cpu.cycles = 0;
    cpu.ime = 0;
    cpu.halted = false;




    // Initialize MMU addresses
    init_mmu_addresses(&mmu_addresses);
    // Set default LCD registers
    write_byte(0xFF40, 0x91); // LCDC: LCD on, BG on
    write_byte(0xFF42, 0x00); // SCY
    write_byte(0xFF43, 0x00); // SCX
    write_byte(0xFF44, 0x00); // LY
    write_byte(0xFF47, 0xE4); // BGP: White to Black
    // Initialize PPU state
    ppu_init(&ppu, &mmu_addresses);
    //set_ppu_mode(&mmu_addresses, 2);

    InitWindow(1500, 1000, "Rayboy Emulator");
    while (!WindowShouldClose())
    {
        //update_input();
        for (int i= 0; i < 69905; i++){
            int cycles = emulate_cycle(&cpu);
            //ppu_step(&ppu, cycles, &mmu_addresses);
            printf("PC: %04X\n", cpu.pc);
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        display_vram(&ppu, &mmu_addresses, &x, &y, &dev);
        //test_frame(&ppu);
        //debug_render(&ppu);
        //display_frame(&ppu);
        EndDrawing();

    }

    CloseWindow();
    return 0;
}
