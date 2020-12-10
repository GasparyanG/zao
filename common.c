#include "common.h"

uint16_t bytesFusion(uint8_t left, uint8_t right) {
    uint16_t combined = left;
    combined << 8;
    combined |= right;

    return combined;
}