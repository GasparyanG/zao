#ifndef ZAO_DEBUG_H
#define ZAO_DEBUG_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "chunk.h"
#include "compiler.h"

void displayInstruction(uint8_t* ip);

#endif