#include <ppu.h>

/* CPU <-> PPU Shared Registers */
#define CPU_MEM_PPU_REG_OAM_ADDR 0x2003

void nes_ppu_init(nes_ppu_t *nes_ppu, nes_memmap_t *memmap)
{   
    nes_ppu->memmap = memmap;

    nes_ppu->regs.ctrl = 0;
    nes_ppu->regs.mask = 0;
    nes_ppu->regs.status = 0;
 
    nes_ppu->current_scan_line = 0;   
    nes_ppu->address_latch = 0;

    nes_ppu->ppu_scroll_x = 0;
    nes_ppu->ppu_scroll_y = 0;
    nes_ppu->ppu_oam_addr = 0;
    nes_ppu->ppu_vram_addr = 0;
    nes_ppu->ppu_read_buffer = 0;
    nes_ppu->ppu_byte_cache = 0;
}

void nes_ppu_write_oam_data(nes_memmap_t *memmap, uint8_t val)
{
    uint8_t addr = *memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REG_OAM_ADDR];
    memmap->ppu_mem_map.oam_data[addr] = val;

    /* increment addr */
    *memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REG_OAM_ADDR] = (*memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REG_OAM_ADDR]+1)%PPU_MEM_OAMD_SIZE;
}

uint8_t nes_ppu_read_oam_data(nes_memmap_t *memmap)
{
    uint8_t addr = *memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REG_OAM_ADDR];
    return memmap->ppu_mem_map.oam_data[addr];
}
