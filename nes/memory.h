#include <stdint.h>

typedef struct nes_memmap_s
{
    /* Only OxC807 bytes are actually used by the nes:
     * 0xffff−0x800−0x800−0x800−0x1ff8 = 0xC807
     */
    uint8_t mem_phys[0xC807];

    /* This will be the 'virtual' mem map to the physical memory*/
    uint8_t *mem_virt[0xFFFF];

} nes_memmap_t;

void nes_memmap_init(nes_memmap_t *memmap);
