#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <memory_controller.h>

#define ERR_FILE_NOT_EXIST              (1)
#define ERR_NES_FILE_HEADER_READ_FAILED (2)
#define ERR_MEMORY_ALLOCATE_FAILED      (3)
#define ERR_PRG_ROM_LOAD_FAILED         (4)
#define ERR_CHR_ROM_LOAD_FAILED         (5)

typedef struct nes_cartridge_s
{
    uint8_t header[16];
    int prg_rom_size;
    int chr_rom_size;
    int prg_ram_size;
    uint8_t *prg_rom;
    uint8_t *chr_rom;

	/* New interface */
	nes_mem_td *nes_mem;

} nes_cartridge_t;

int nes_cart_load_rom(nes_cartridge_t *nes_cart, char *rom);
void nes_cart_init(nes_cartridge_t *nes_cart, nes_mem_td *nes_mem);
void nes_cart_print_rom_metadata(nes_cartridge_t *nes_cart);

#endif