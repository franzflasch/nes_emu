#include <stdio.h>
#include <memory.h>

#define MEM_INTERNAL_RAM_OFFSET         0x0000
#define MEM_INTERNAL_RAM_SIZE           0x0800

#define MEM_INTERNAL_RAM_MIRROR1_OFFSET 0x0800
#define MEM_INTERNAL_RAM_MIRROR1_SIZE   0x0800

#define MEM_INTERNAL_RAM_MIRROR2_OFFSET 0x1000
#define MEM_INTERNAL_RAM_MIRROR2_SIZE   0x0800

#define MEM_INTERNAL_RAM_MIRROR3_OFFSET 0x1800
#define MEM_INTERNAL_RAM_MIRROR3_SIZE   0x0800

#define MEM_PPU_REGISTER_OFFSET         0x2000
#define MEM_PPU_REGISTER_SIZE           0x0008

#define MEM_PPU_MIRRORS_OFFSET          0x2008
#define MEM_PPU_MIRRORS_SIZE            0x1FF8
#define MEM_PPU_MIRRORS_REPEAT          0x0008

#define MEM_APU_REGISTER_OFFSET         0x4000
#define MEM_APU_REGISTER_SIZE           0x0018

#define MEM_APU_IO_REGISTER_OFFSET      0x4018
#define MEM_APU_IO_REGISTER_SIZE        0x0008

#define MEM_CRTRDG_REGISTER_OFFSET      0x4020
#define MEM_CRTRDG_REGISTER_SIZE        0xBFE0


void nes_memmap_init(nes_memmap_t *memmap)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t offset = 0;

    printf("begin\n");

    /* map internal ram */
    offset = 0;
    for(i=MEM_INTERNAL_RAM_OFFSET;i<(MEM_INTERNAL_RAM_OFFSET+MEM_INTERNAL_RAM_SIZE);i++)
    {
        printf("INTRAM virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }


    /* map internal ram mirror 1 */
    offset = 0;
    for(i=MEM_INTERNAL_RAM_MIRROR1_OFFSET;i<(MEM_INTERNAL_RAM_MIRROR1_OFFSET+MEM_INTERNAL_RAM_MIRROR1_SIZE);i++)
    {
        printf("INTRAM_MIRROR1 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map internal ram mirror 2 */
    offset = 0;
    for(i=MEM_INTERNAL_RAM_MIRROR2_OFFSET;i<(MEM_INTERNAL_RAM_MIRROR2_OFFSET+MEM_INTERNAL_RAM_MIRROR2_SIZE);i++)
    {
        printf("INTRAM_MIRROR2 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map internal ram mirror 3 */
    offset = 0;
    for(i=MEM_INTERNAL_RAM_MIRROR3_OFFSET;i<(MEM_INTERNAL_RAM_MIRROR3_OFFSET+MEM_INTERNAL_RAM_MIRROR3_SIZE);i++)
    {
        printf("INTRAM_MIRROR3 virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map PPU registers */
    offset = MEM_INTERNAL_RAM_SIZE;
    for(i=MEM_PPU_REGISTER_OFFSET;i<(MEM_PPU_REGISTER_OFFSET+MEM_PPU_REGISTER_SIZE);i++)
    {
        printf("PPU virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map PPU mirror registers */
    offset = MEM_INTERNAL_RAM_SIZE;
    for(i=MEM_PPU_MIRRORS_OFFSET,j=0;i<(MEM_PPU_MIRRORS_OFFSET+MEM_PPU_MIRRORS_SIZE);i++,j++)
    {
        printf("PPU MIRROR virt: %x phys: %x\n", i, offset+(j%MEM_PPU_MIRRORS_REPEAT));
        memmap->mem_virt[i] = &memmap->mem_phys[offset+(j%MEM_PPU_MIRRORS_REPEAT)];
    }

    /* map APU registers */
    offset = MEM_INTERNAL_RAM_SIZE + MEM_PPU_REGISTER_SIZE;
    for(i=MEM_APU_REGISTER_OFFSET;i<(MEM_APU_REGISTER_OFFSET+MEM_APU_REGISTER_SIZE);i++)
    {
        printf("APU virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map APU IO registers */
    offset = MEM_INTERNAL_RAM_SIZE + MEM_PPU_REGISTER_SIZE + MEM_APU_REGISTER_SIZE;
    for(i=MEM_APU_IO_REGISTER_OFFSET;i<(MEM_APU_IO_REGISTER_OFFSET+MEM_APU_IO_REGISTER_SIZE);i++)
    {
        printf("APU IO virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }

    /* map CARTRIDGE space */
    offset = MEM_INTERNAL_RAM_SIZE + MEM_PPU_REGISTER_SIZE + MEM_APU_REGISTER_SIZE + MEM_APU_IO_REGISTER_SIZE;
    for(i=MEM_CRTRDG_REGISTER_OFFSET;i<(MEM_CRTRDG_REGISTER_OFFSET+MEM_CRTRDG_REGISTER_SIZE);i++)
    {
        printf("CRTRDG virt: %x phys: %x\n", i, offset);
        memmap->mem_virt[i] = &memmap->mem_phys[offset];
        offset++;
    }
}
