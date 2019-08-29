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

uint16_t cpu_memory_read_byte(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    return (memmap->cpu_memory[addr]); 
}

uint16_t cpu_memory_read_word(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    return cpu_memory_read_byte(memmap, addr, 0) + (cpu_memory_read_byte(memmap, addr + 1, 0) << 8);
}

uint16_t cpu_memory_write_byte(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    memmap->cpu_memory[addr] = data;
    return 0;
}

uint16_t cpu_memory_write_word(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    cpu_memory_write_byte(memmap, addr, data & 0xFF);
    cpu_memory_write_byte(memmap, addr + 1, data >> 8);
    return 0;
}

// static uint16_t (*ppu_access_funcs[ACCESS_FUNC_MAX])(nes_mem_td *memmap, uint16_t addr, uint16_t data) = 
// {
//     NULL,
//     NULL,
//     NULL,
//     NULL
// };

// static uint16_t (*controller_access_funcs[ACCESS_FUNC_MAX])(nes_mem_td *memmap, uint16_t addr, uint16_t data) = 
// {
//     NULL,
//     NULL,
//     NULL,
//     NULL
// };

static uint16_t (*cpu_access_funcs[ACCESS_FUNC_MAX])(nes_mem_td *memmap, uint16_t addr, uint16_t data) = 
{
    cpu_memory_write_word,
    cpu_memory_write_byte,
    cpu_memory_read_word,
    cpu_memory_read_byte
};

uint16_t memory_access(nes_mem_td *memmap, uint16_t addr, uint16_t data, uint8_t access_type)
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);

    if(check_is_ppu_reg(actual_addr))
    {
        return 0;
        //return ppu_access_funcs[access_type](memmap, actual_addr, data);
    }
    else if(check_is_controller_reg(actual_addr))
    {
        return 0;
        //return controller_access_funcs[access_type](memmap, actual_addr, data);
    }
    else
    {
        return cpu_access_funcs[access_type](memmap, actual_addr, data);
    }
}
