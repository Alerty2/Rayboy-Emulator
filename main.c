#include "ppu.h"
#include "input.h"

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

int main(int argc, char* argv[])
{
    memory[0xFF40] = 0x91;  // LCD ON + BG ON + BG Tile Map 0x9C00
    memory[0xFF47] = 0xE4;  // BGP: 11 10 01 00

    // Check if ROM was provided
    if (argc < 2) {
        printf("Usage: %s <rom_file.gb>\n", argv[0]);
        return 1;
    }

    // Load ROM from provided path
    if (load_rom(memory, argv[1]) < 0) {
        printf("Failed to load ROM: %s\n", argv[1]);
        return 1;
    }
    printf("Tile data base: %02X %02X %02X %02X\n",
           memory[0x8000], memory[0x8001], memory[0x8002], memory[0x8003]);


    // Initialize CPU state
    cpu.pc = 0x100;
    cpu.sp = 0xFFFE;
    cpu.af.AF = 0x01B0;
    cpu.bc.BC = 0x0013;
    cpu.de.DE = 0x00D8;
    cpu.hl.HL = 0x014D;
    cpu.cycles = 0;
    cpu.ime = 0;




    // Initialize MMU addresses
    init_mmu_addresses(&mmu_addresses);
    // Set default LCD registers
    memory[0xFF40] = 0x91;  // LCDC: LCD on, BG on
    memory[0xFF42] = 0x00;  // SCY
    memory[0xFF43] = 0x00;  // SCX
    memory[0xFF44] = 0x00;  // LY
    memory[0xFF47] = 0xE4;  // BGP: White to Black
    // Initialize PPU state
    ppu_init(&ppu);
    set_ppu_mode(memory, &mmu_addresses, 2);

    InitWindow(160, 144, "Rayboy Emulator");

    while (!WindowShouldClose())
    {
        update_input(memory);
        for (int i= 0; i < 69905/4; i++){
            int cycles = emulate_cycle(memory, &cpu);
            printf("LCDC: %02X | LY: %02X | Mode: %d\n", memory[0xFF40], memory[0xFF44], ppu.mode);
            ppu_step(&ppu, cycles, memory, &mmu_addresses);

            printf("PC: %04X\n", cpu.pc);
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        //test_frame(&ppu);
        display_frame(&ppu);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
