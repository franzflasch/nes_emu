#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <memory.h>

uint8_t nes_key_state(uint8_t b);
void controller_run(nes_memmap_t *nes_mem);

#endif
