#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define CPU_MEM_PHYS_SIZE 0xC808
#define CPU_MEM_VIRT_SIZE 0x10000

#define PPU_MEM_PHYS_SIZE 0x3020
#define PPU_MEM_VIRT_SIZE 0x4000
#define PPU_MEM_OAMD_SIZE 0x0100

typedef struct nes_cpu_memmap_s
{
    /* Only OxC808 bytes are actually used by the nes:
     * 0x10000−0x800−0x800−0x800−0x1ff8 = 0xC808
     */
    uint8_t mem_phys[CPU_MEM_PHYS_SIZE];

    /* This will be the 'virtual' mem map to the physical memory*/
    uint8_t *mem_virt[CPU_MEM_VIRT_SIZE];

} nes_cpu_memmap_t;

typedef struct nes_ppu_memmap_s
{
    /* Only 0x3100 bytes are actually used by the ppu:
     * 0x4000−0xF00 = 0x3100
     */
    uint8_t mem_phys[PPU_MEM_PHYS_SIZE];

    /* This will be the 'virtual' mem map to the physical memory*/
    uint8_t *mem_virt[PPU_MEM_VIRT_SIZE];

    /* OAM Data Memory */
    uint8_t oam_data[PPU_MEM_OAMD_SIZE];

} nes_ppu_memmap_t;

typedef struct nes_memmap_s
{
    nes_cpu_memmap_t cpu_mem_map;
    nes_ppu_memmap_t ppu_mem_map;

} nes_memmap_t;

void nes_memmap_init(nes_memmap_t *memmap);

#endif