#include <stdio.h>
#include <memory.h>
#include <string.h>

#define DEBUG 0

#define debug_print(fmt, ...) \
            do { if (DEBUG) printf(fmt, __VA_ARGS__); } while (0)

static void nes_cpu_memmap_init(nes_cpu_memmap_t *memmap)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t offset = 0;

    memset(memmap, 0, sizeof(nes_cpu_memmap_t));

    /* map internal ram */
    offset = 0;
    for(i=CPU_MEM_INTERNAL_RAM_OFFSET;i<(CPU_MEM_INTERNAL_RAM_OFFSET+CPU_MEM_INTERNAL_RAM_SIZE);i++)
    {
        debug_print("INTRAM virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }


    /* map internal ram mirror 1 */
    offset = 0;
    for(i=CPU_MEM_INTERNAL_RAM_MIRROR1_OFFSET;i<(CPU_MEM_INTERNAL_RAM_MIRROR1_OFFSET+CPU_MEM_INTERNAL_RAM_MIRROR1_SIZE);i++)
    {
        debug_print("INTRAM_MIRROR1 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map internal ram mirror 2 */
    offset = 0;
    for(i=CPU_MEM_INTERNAL_RAM_MIRROR2_OFFSET;i<(CPU_MEM_INTERNAL_RAM_MIRROR2_OFFSET+CPU_MEM_INTERNAL_RAM_MIRROR2_SIZE);i++)
    {
        debug_print("INTRAM_MIRROR2 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map internal ram mirror 3 */
    offset = 0;
    for(i=CPU_MEM_INTERNAL_RAM_MIRROR3_OFFSET;i<(CPU_MEM_INTERNAL_RAM_MIRROR3_OFFSET+CPU_MEM_INTERNAL_RAM_MIRROR3_SIZE);i++)
    {
        debug_print("INTRAM_MIRROR3 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map PPU registers */
    offset = CPU_MEM_INTERNAL_RAM_SIZE;
    for(i=CPU_MEM_PPU_REGISTER_OFFSET;i<(CPU_MEM_PPU_REGISTER_OFFSET+CPU_MEM_PPU_REGISTER_SIZE);i++)
    {
        debug_print("PPU virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map PPU mirror registers */
    offset = CPU_MEM_INTERNAL_RAM_SIZE;
    for(i=CPU_MEM_PPU_MIRRORS_OFFSET,j=0;i<(CPU_MEM_PPU_MIRRORS_OFFSET+CPU_MEM_PPU_MIRRORS_SIZE);i++,j++)
    {
        debug_print("PPU MIRROR virt: %x phys: %x\n", i, offset+(j%CPU_MEM_PPU_MIRRORS_REPEAT));
        memmap->mem_virt[i] = &memmap->mem_phys[offset+(j%CPU_MEM_PPU_MIRRORS_REPEAT)];
    }

    /* map APU registers */
    offset = CPU_MEM_INTERNAL_RAM_SIZE + CPU_MEM_PPU_REGISTER_SIZE;
    for(i=CPU_MEM_APU_REGISTER_OFFSET;i<(CPU_MEM_APU_REGISTER_OFFSET+CPU_MEM_APU_REGISTER_SIZE);i++)
    {
        debug_print("APU virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map APU IO registers */
    offset = CPU_MEM_INTERNAL_RAM_SIZE + CPU_MEM_PPU_REGISTER_SIZE + CPU_MEM_APU_REGISTER_SIZE;
    for(i=CPU_MEM_APU_IO_REGISTER_OFFSET;i<(CPU_MEM_APU_IO_REGISTER_OFFSET+CPU_MEM_APU_IO_REGISTER_SIZE);i++)
    {
        debug_print("APU IO virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map CARTRIDGE space */
    offset = CPU_MEM_INTERNAL_RAM_SIZE + CPU_MEM_PPU_REGISTER_SIZE + CPU_MEM_APU_REGISTER_SIZE + CPU_MEM_APU_IO_REGISTER_SIZE;
    for(i=CPU_MEM_CRTRDG_REGISTER_OFFSET;i<(CPU_MEM_CRTRDG_REGISTER_OFFSET+CPU_MEM_CRTRDG_REGISTER_SIZE);i++)
    {
        debug_print("CRTRDG virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }
}

static void nes_ppu_memmap_init(nes_ppu_memmap_t *memmap)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t offset = 0;

    memset(memmap, 0, sizeof(nes_ppu_memmap_t));

    /* map pattern table 0 */
    offset = 0;
    for(i=PPU_MEM_PATTERN_TABLE0_OFFSET;i<(PPU_MEM_PATTERN_TABLE0_OFFSET+PPU_MEM_PATTERN_TABLE0_SIZE);i++)
    {
        debug_print("PPU_MEM_PATTERN_TABLE0 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map pattern table 1 */
    offset = PPU_MEM_PATTERN_TABLE0_SIZE;
    for(i=PPU_MEM_PATTERN_TABLE1_OFFSET;i<(PPU_MEM_PATTERN_TABLE1_OFFSET+PPU_MEM_PATTERN_TABLE1_SIZE);i++)
    {
        debug_print("PPU_MEM_PATTERN_TABLE1 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map name table 0 */
    offset = PPU_MEM_PATTERN_TABLE0_SIZE + PPU_MEM_PATTERN_TABLE1_SIZE;
    for(i=PPU_MEM_NAME_TABLE0_OFFSET;i<(PPU_MEM_NAME_TABLE0_OFFSET+PPU_MEM_NAME_TABLE0_SIZE);i++)
    {
        debug_print("PPU_MEM_NAME_TABLE0 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map name table 1 */
    offset = PPU_MEM_PATTERN_TABLE0_SIZE + PPU_MEM_PATTERN_TABLE1_SIZE + PPU_MEM_NAME_TABLE0_SIZE;
    for(i=PPU_MEM_NAME_TABLE1_OFFSET;i<(PPU_MEM_NAME_TABLE1_OFFSET+PPU_MEM_NAME_TABLE1_SIZE);i++)
    {
        debug_print("PPU_MEM_NAME_TABLE1 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map name table 2 */
    offset = PPU_MEM_PATTERN_TABLE0_SIZE + PPU_MEM_PATTERN_TABLE1_SIZE + PPU_MEM_NAME_TABLE0_SIZE + PPU_MEM_NAME_TABLE1_SIZE;
    for(i=PPU_MEM_NAME_TABLE2_OFFSET;i<(PPU_MEM_NAME_TABLE2_OFFSET+PPU_MEM_NAME_TABLE2_SIZE);i++)
    {
        debug_print("PPU_MEM_NAME_TABLE2 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map name table 3 */
    offset = PPU_MEM_PATTERN_TABLE0_SIZE + PPU_MEM_PATTERN_TABLE1_SIZE + PPU_MEM_NAME_TABLE0_SIZE + PPU_MEM_NAME_TABLE1_SIZE + PPU_MEM_NAME_TABLE2_SIZE;
    for(i=PPU_MEM_NAME_TABLE3_OFFSET;i<(PPU_MEM_NAME_TABLE3_OFFSET+PPU_MEM_NAME_TABLE3_SIZE);i++)
    {
        debug_print("PPU_MEM_NAME_TABLE3 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map PPU mirror registers */
    offset = PPU_MEM_PATTERN_TABLE0_SIZE + PPU_MEM_PATTERN_TABLE1_SIZE;
    for(i=PPU_MEM_NAME_TABLE_MIRRORS_OFFSET,j=0;i<(PPU_MEM_NAME_TABLE_MIRRORS_OFFSET+PPU_MEM_NAME_TABLE_MIRRORS_SIZE);i++,j++)
    {
        debug_print("PPU_MEM_NAME_TABLE MIRROR virt: %x phys: %x\n", i, offset+(j%PPU_MEM_NAME_TABLE_MIRRORS_REPEAT));
        memmap->mem_virt[i] = &memmap->mem_phys[offset+(j%PPU_MEM_NAME_TABLE_MIRRORS_REPEAT)];
    }

    /* map palette ram */
    offset = PPU_MEM_PATTERN_TABLE0_SIZE + PPU_MEM_PATTERN_TABLE1_SIZE + PPU_MEM_NAME_TABLE0_SIZE + PPU_MEM_NAME_TABLE1_SIZE + PPU_MEM_NAME_TABLE2_SIZE + PPU_MEM_NAME_TABLE3_SIZE;
    for(i=PPU_MEM_PALETTE_RAM_OFFSET;i<(PPU_MEM_PALETTE_RAM_OFFSET+PPU_MEM_PALETTE_RAM_SIZE);i++)
    {
        debug_print("PPU_MEM_PALETTE_RAM virt: %x phys: %x\n", i, offset);

        /* Some regs here are internally mirrored */
        if(i==0x3F10) memmap->mem_virt[i] = memmap->mem_virt[0x3F00];
        else if(i==0x3F14) memmap->mem_virt[i] = memmap->mem_virt[0x3F04];
        else if (i==0x3F18) memmap->mem_virt[i] = memmap->mem_virt[0x3F08];
        else if (i==0x3F1C) memmap->mem_virt[i] = memmap->mem_virt[0x3F0C];
        else memmap->mem_virt[i] = &memmap->mem_phys[offset];

        offset++;
    }

    /* map palette ram mirrors */
    offset = PPU_MEM_PATTERN_TABLE0_SIZE + PPU_MEM_PATTERN_TABLE1_SIZE + PPU_MEM_NAME_TABLE0_SIZE + PPU_MEM_NAME_TABLE1_SIZE + PPU_MEM_NAME_TABLE2_SIZE + PPU_MEM_NAME_TABLE3_SIZE;
    for(i=PPU_MEM_PALETTE_RAM_MIRRORS_OFFSET,j=0;i<(PPU_MEM_PALETTE_RAM_MIRRORS_OFFSET+PPU_MEM_PALETTE_RAM_MIRRORS_SIZE);i++,j++)
    {
        debug_print("PPU_PALETTE_RAM_MIRRORS virt: %x phys: %x\n", i, offset+(j%PPU_MEM_PALETTE_RAM_MIRRORS_REPEAT));
        memmap->mem_virt[i] = &memmap->mem_phys[offset+(j%PPU_MEM_PALETTE_RAM_MIRRORS_REPEAT)];
    }
}

void nes_memmap_init(nes_memmap_t *memmap)
{
    nes_ppu_memmap_init(&memmap->ppu_mem_map);
    nes_cpu_memmap_init(&memmap->cpu_mem_map);
    memmap->last_reg_accessed = 0;
    memmap->last_reg_read_write = 0;
}

