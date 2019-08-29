#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <memory_controller.h>

#define PPU_STATUS_FRAME_READY (1 << 0)
#define PPU_STATUS_NMI (1 << 1)
#define PPU_STATUS_OAM_ACCESS (1 << 2)

typedef struct nes_ppu_s
{
	uint16_t current_scan_line;
    uint16_t current_pixel;

    nes_mem_td *nes_memory;

    uint32_t screen_bitmap[256*240];

} nes_ppu_t;

/* NTSC color pallete */
typedef struct ppu_color_pallete_s
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

} ppu_color_pallete_t;

// void nes_ppu_write_oam_data(nes_memmap_t *memmap, uint8_t val);
// uint8_t nes_ppu_read_oam_data(nes_memmap_t *memmap);

uint16_t ppu_reg_access(nes_mem_td *memmap, uint16_t addr, uint16_t data, uint8_t access_type);
void nes_ppu_init(nes_ppu_t *nes_ppu, nes_mem_td *nes_memory);
uint8_t nes_ppu_run(nes_ppu_t *nes_ppu, uint32_t cpu_cycles);
void nes_ppu_dump_regs(nes_ppu_t *nes_ppu);

#endif