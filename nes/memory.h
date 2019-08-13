#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

/* CPU MEMORY */
#define CPU_MEM_INTERNAL_RAM_OFFSET         0x0000
#define CPU_MEM_INTERNAL_RAM_SIZE           0x0800

#define CPU_MEM_INTERNAL_RAM_MIRROR1_OFFSET 0x0800
#define CPU_MEM_INTERNAL_RAM_MIRROR1_SIZE   0x0800

#define CPU_MEM_INTERNAL_RAM_MIRROR2_OFFSET 0x1000
#define CPU_MEM_INTERNAL_RAM_MIRROR2_SIZE   0x0800

#define CPU_MEM_INTERNAL_RAM_MIRROR3_OFFSET 0x1800
#define CPU_MEM_INTERNAL_RAM_MIRROR3_SIZE   0x0800

#define CPU_MEM_PPU_REGISTER_OFFSET         0x2000
#define CPU_MEM_PPU_REGISTER_SIZE           0x0008

#define CPU_MEM_PPU_MIRRORS_OFFSET          0x2008
#define CPU_MEM_PPU_MIRRORS_SIZE            0x1FF8
#define CPU_MEM_PPU_MIRRORS_REPEAT          0x0008

#define CPU_MEM_APU_REGISTER_OFFSET         0x4000
#define CPU_MEM_APU_REGISTER_SIZE           0x0018

#define CPU_MEM_APU_IO_REGISTER_OFFSET      0x4018
#define CPU_MEM_APU_IO_REGISTER_SIZE        0x0008

#define CPU_MEM_CRTRDG_REGISTER_OFFSET      0x4020
#define CPU_MEM_CRTRDG_REGISTER_SIZE        0xBFE0

/* PPU MEMORY */
#define PPU_MEM_PATTERN_TABLE0_OFFSET       0x0000
#define PPU_MEM_PATTERN_TABLE0_SIZE         0x1000

#define PPU_MEM_PATTERN_TABLE1_OFFSET       0x1000
#define PPU_MEM_PATTERN_TABLE1_SIZE         0x1000

#define PPU_MEM_NAME_TABLE0_OFFSET          0x2000
#define PPU_MEM_NAME_TABLE0_SIZE            0x0400

#define PPU_MEM_NAME_TABLE1_OFFSET          0x2400
#define PPU_MEM_NAME_TABLE1_SIZE            0x0400

#define PPU_MEM_NAME_TABLE2_OFFSET          0x2800
#define PPU_MEM_NAME_TABLE2_SIZE            0x0400

#define PPU_MEM_NAME_TABLE3_OFFSET          0x2C00
#define PPU_MEM_NAME_TABLE3_SIZE            0x0400

#define PPU_MEM_NAME_TABLE_MIRRORS_OFFSET   0x3000
#define PPU_MEM_NAME_TABLE_MIRRORS_SIZE     0x0F00
#define PPU_MEM_NAME_TABLE_MIRRORS_REPEAT   0x0400

#define PPU_MEM_PALETTE_RAM_OFFSET          0x3F00
#define PPU_MEM_PALETTE_RAM_SIZE            0x0020

#define PPU_MEM_PALETTE_RAM_MIRRORS_OFFSET  0x3F20
#define PPU_MEM_PALETTE_RAM_MIRRORS_SIZE    0x00E0
#define PPU_MEM_PALETTE_RAM_MIRRORS_REPEAT  0x0020

/* MEM Defines needed for emulation */
#define CPU_MEM_PHYS_SIZE 0xC808
#define CPU_MEM_VIRT_SIZE 0x10000

#define PPU_MEM_PHYS_SIZE 0x3020
#define PPU_MEM_VIRT_SIZE 0x4000
#define PPU_MEM_OAMD_SIZE 0x0100

#define PPU_REG_ACCESS_READ 1
#define PPU_REG_ACCESS_WRITE 2

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
    uint16_t ppu_last_reg_accessed;
    uint16_t ppu_last_reg_read_write;

} nes_memmap_t;

void nes_memmap_init(nes_memmap_t *memmap);

#endif