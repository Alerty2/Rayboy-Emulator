#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#define ROM_SIZE 0x8000

int load_rom(uint8_t memory[], const char *filename);
