#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <memory.h>

typedef struct ppu_regs_s
{
    // uint8_t name_table_address : 2;
    // uint8_t vertical_write : 1;
    // uint8_t sprite_pattern_table_addr : 1;
    // uint8_t screen_pattern_table_addr : 1;
    // uint8_t sprite_size : 1;
    // uint8_t unused : 1;
    // uint8_t vblank_enabled : 1;

	uint8_t ctrl;


    // uint8 unused : 1;
    // uint8 screen_mask : 1;
    // uint8 sprite_mask : 1;
    // uint8 screen_enabled : 1;
    // uint8 sprites_enabled : 1;
    // uint8 screen_red_tint : 1;
    // uint8 screen_green_tint : 1;
    // uint8 screen_blue_tint : 1;

    uint8_t mask;


    // uint8 unused : 5;
    // uint8 sprite_overflow_bit : 1;
    // uint8 sprite_zero_hit : 1;
    // uint8 vblank_flag : 1;    

    uint8_t status;


} ppu_regs_t;

typedef struct nes_ppu_s
{
	nes_memmap_t *memmap;
	ppu_regs_t regs;

	uint32_t current_scan_line;
    uint8_t ppu_scroll_x;
    uint8_t ppu_scroll_y;
    uint8_t ppu_oam_addr;
    uint8_t ppu_read_buffer;
    uint16_t ppu_vram_addr;
    uint8_t ppu_byte_cache;

    uint8_t address_latch;

} nes_ppu_t;

void nes_ppu_init(nes_ppu_t *nes_ppu, nes_memmap_t *memmap);
void nes_ppu_write_oam_data(nes_memmap_t *memmap, uint8_t val);
uint8_t nes_ppu_read_oam_data(nes_memmap_t *memmap);

#endif