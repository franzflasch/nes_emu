#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

uint8_t nes_key_state(uint8_t b);
uint8_t psg_io_read(void);
void psg_io_write(uint8_t data);
//void controller_run(nes_memmap_t *nes_mem);

#endif
