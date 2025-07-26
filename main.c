#include "emulation.h"
#include "input.h"

// Define the RAM
uint8_t wRAM[8192];
uint8_t vRAM[8192];

// Define the memory
uint8_t memory[0x10000];

// Define CPU
CPU cpu;

// Input state
uint8_t input_state;

int main(int argc, char* argv[])
{
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

    // Initialize CPU state
    cpu.pc = 0x100;
    cpu.sp = 0xFFFE;
    cpu.af.AF = 0x01B0;
    cpu.bc.BC = 0x0013;
    cpu.de.DE = 0x00D8;
    cpu.hl.HL = 0x014D;
    cpu.cycles = 0;

    InitWindow(800, 600, "Rayboy Emulator");

    while (!WindowShouldClose())
    {
        update_input(memory);
        for (int i= 0; i < 69905/4; i++){
            emulate_cycle(memory, &cpu);
            printf("PC: %04X\n", cpu.pc);
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Currently no GUI", 190, 200, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
