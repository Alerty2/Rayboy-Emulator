gcc main.c emulation.c flags.c input.c instructions.c loader.c ppu.c mmu.c -o rayboy -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -fsanitize=address -g
