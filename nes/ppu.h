#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <memory.h>

#define PPU_STATUS_FRAME_READY (1 << 0)
#define PPU_STATUS_NMI (1 << 1)

typedef struct __attribute__((packed)) ppu_regs_s
{
    // $2000
    // uint8_t name_table_address : 2;
    // uint8_t vertical_write : 1;
    // uint8_t sprite_pattern_table_addr : 1;
    // uint8_t screen_pattern_table_addr : 1;
    // uint8_t sprite_size : 1;
    // uint8_t unused : 1;
    // uint8_t vblank_enabled : 1;
	uint8_t ctrl;

    // $2001
    // uint8 unused : 1;
    // uint8 screen_mask : 1;
    // uint8 sprite_mask : 1;
    // uint8 screen_enabled : 1;
    // uint8 sprites_enabled : 1;
    // uint8 screen_red_tint : 1;
    // uint8 screen_green_tint : 1;
    // uint8 screen_blue_tint : 1;
    uint8_t mask;

    // $2002
    // uint8 unused : 5;
    // uint8 sprite_overflow_bit : 1;
    // uint8 sprite_zero_hit : 1;
    // uint8 vblank_flag : 1;    
    uint8_t status;

    // $2003
    uint8_t oamaddr;

    // $2004
    uint8_t oamdata;

    // $2005
    uint8_t scroll;

    // $2006
    uint8_t addr;

    // $2007
    uint8_t data;

} ppu_regs_t;

typedef struct nes_ppu_s
{
	nes_memmap_t *memmap;
	ppu_regs_t *regs;

    uint8_t addr_access_cycle;

    uint16_t curr_ppu_data_address;

	uint16_t current_scan_line;
    uint16_t current_pixel;

    uint32_t screen_bitmap[256*240];

} nes_ppu_t;

/* NTSC color pallete */
typedef struct ppu_color_pallete_s
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

} ppu_color_pallete_t;

void nes_ppu_write_oam_data(nes_memmap_t *memmap, uint8_t val);
uint8_t nes_ppu_read_oam_data(nes_memmap_t *memmap);

void nes_ppu_init(nes_ppu_t *nes_ppu, nes_memmap_t *memmap);
uint8_t nes_ppu_run(nes_ppu_t *nes_ppu);
void nes_ppu_dump_regs(nes_ppu_t *nes_ppu);

#endif