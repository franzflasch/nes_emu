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

static uint8_t check_is_ppu_reg(uint16_t addr)
{
    if((addr >= CPU_MEM_PPU_REGISTER_OFFSET) && (addr < CPU_MEM_PPU_REGISTER_OFFSET+CPU_MEM_PPU_REGISTER_SIZE))
    {
        return 1;
    }
    return 0;
}

static uint8_t check_is_controller_reg(uint16_t addr)
{
    if((addr == CPU_CONTROLLER1_REGISTER))
    {
        return 1;
    }
    return 0;
}

uint8_t memory_read_byte(nes_cpu_mem_td *memmap, uint16_t addr) 
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);

    if(check_is_ppu_reg(actual_addr))
    {
        // FIXME ppu reg access;
        return 0;
    }
    else if (check_is_controller_reg(actual_addr))
    {
        // FIXME controller reg access;
        return 0;
    }
    else
        return (memmap->memory[actual_addr]); 
}

uint16_t memory_read_word(nes_cpu_mem_td *memmap, uint16_t addr) 
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);

    if(check_is_ppu_reg(actual_addr))
    {
        // FIXME ppu reg access;
        return 0;
    }
    else if (check_is_controller_reg(actual_addr))
    {
        // FIXME controller reg access;
        return 0;
    }
    else
        return memory_read_byte(memmap, actual_addr) + (memory_read_byte(memmap, actual_addr + 1) << 8);
}

void memory_write_byte(nes_cpu_mem_td *memmap, uint16_t addr, uint8_t data) 
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);

    if(check_is_ppu_reg(actual_addr))
    {
        // FIXME ppu reg access;
    }
    else if (check_is_controller_reg(actual_addr))
    {
        // FIXME controller reg access;
    }
    else
        memmap->memory[actual_addr] = data;
}

void memory_write_word(nes_cpu_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);

    if(check_is_ppu_reg(actual_addr))
    {
        // FIXME ppu reg access;
    }
    else if (check_is_controller_reg(actual_addr))
    {
        // FIXME controller reg access;
    }
    else
    {
        memory_write_byte(memmap, actual_addr, data & 0xFF);
        memory_write_byte(memmap, actual_addr + 1, data >> 8);
    }
}

void cpu_memory_init(nes_cpu_mem_td *memmap)
{
    memset(memmap, 0, sizeof(nes_cpu_mem_td));
}
