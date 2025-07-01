#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#define ROM_SIZE 0x8000

int load_rom(uint8_t memory[], const char *filename){
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("ROM couldn't be opened\n");
        return -1;
    }

    size_t bytesRead = fread(memory, 1, ROM_SIZE, f);
    fclose(f);

    if (bytesRead == 0) {
        printf("Empty ROM\n");
        return -1;
    }

    printf("ROM loaded!, %zu bytes\n", bytesRead);
    return 0;
}
