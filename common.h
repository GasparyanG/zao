#ifndef ZAO_COMMON_H
#define ZAO_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define ZAO_DEBUGGER_MODE_ON
#define ZAO_TOKEN_DEBUGGER
#define ZAO_GC

uint16_t bytesFusion(uint8_t left, uint8_t right);
void fatalError(const char* message);

#endif