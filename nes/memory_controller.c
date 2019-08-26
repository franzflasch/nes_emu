#include <stdio.h>
#include <nes.h>
#include <memory_controller.h>
#include <string.h>

#define debug_print(fmt, ...) \
            do { if (DEBUG_MEMORY) printf(fmt, __VA_ARGS__); } while (0)

static uint16_t cpu_memory_address_demirror(uint16_t addr)
{
    if(addr < CPU_MEM_PPU_REGISTER_OFFSET)
        return addr % CPU_MEM_INTERNAL_RAM_SIZE;
    else if(addr < CPU_MEM_APU_REGISTER_OFFSET)
        return CPU_MEM_PPU_REGISTER_OFFSET + (addr % CPU_MEM_PPU_REGISTER_SIZE);
    else
        return addr;
}

uint8_t memory_read_byte(nes_cpu_mem_td *memmap, uint16_t addr) 
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);
    return (memmap->memory[actual_addr]); 
}

uint16_t memory_read_word(nes_cpu_mem_td *memmap, uint16_t addr) 
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);
    return memory_read_byte(memmap, actual_addr) + (memory_read_byte(memmap, actual_addr + 1) << 8);
}

void memory_write_byte(nes_cpu_mem_td *memmap, uint16_t addr, uint8_t data) 
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);
    memmap->memory[actual_addr] = data;
}

void memory_write_word(nes_cpu_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);
    memory_write_byte(memmap, actual_addr, data & 0xFF);
    memory_write_byte(memmap, actual_addr + 1, data >> 8);
}

void cpu_memory_init(nes_cpu_mem_td *memmap)
{
    memset(memmap, 0, sizeof(nes_cpu_mem_td));
}
