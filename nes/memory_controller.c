#include <stdio.h>
#include <nes.h>
#include <ppu.h>
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

static uint8_t check_is_prg_rom(uint16_t addr)
{
    if((addr >= CPU_MEM_PRG_LOCATION0) && (addr <= 0xFFFF))
        return 1;
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

static uint16_t cpu_memory_read_byte(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    return (memmap->cpu_memory[addr]); 
}

static uint16_t cpu_memory_read_word(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    return cpu_memory_read_byte(memmap, addr, 0) + (cpu_memory_read_byte(memmap, addr + 1, 0) << 8);
}

static uint16_t cpu_memory_write_byte(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    memmap->cpu_memory[addr] = (uint8_t)data;
    return 0;
}

static uint16_t cpu_memory_write_word(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    cpu_memory_write_byte(memmap, addr, data & 0xFF);
    cpu_memory_write_byte(memmap, addr + 1, data >> 8);
    return 0;
}

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

static uint16_t prg_memory_read_byte(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    return (memmap->cpu_prg_memory[addr]); 
}

static uint16_t prg_memory_read_word(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    return prg_memory_read_byte(memmap, addr, 0) + (prg_memory_read_byte(memmap, addr + 1, 0) << 8);
}

static uint16_t prg_memory_write_byte(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    memmap->cpu_prg_memory[addr] = (uint8_t)data;
    return 0;
}

static uint16_t prg_memory_write_word(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    prg_memory_write_byte(memmap, addr, data & 0xFF);
    prg_memory_write_byte(memmap, addr + 1, data >> 8);
    return 0;
}

// static uint16_t (*controller_access_funcs[ACCESS_FUNC_MAX])(nes_mem_td *memmap, uint16_t addr, uint16_t data) = 
// {
//     NULL,
//     NULL,
//     NULL,
//     NULL
// };

static uint16_t (*prg_access_funcs[ACCESS_FUNC_MAX])(nes_mem_td *memmap, uint16_t addr, uint16_t data) = 
{
    prg_memory_write_word,
    prg_memory_write_byte,
    prg_memory_read_word,
    prg_memory_read_byte
};

uint16_t cpu_memory_access(nes_mem_td *memmap, uint16_t addr, uint16_t data, uint8_t access_type)
{
    uint16_t actual_addr = cpu_memory_address_demirror(addr);

    if(check_is_ppu_reg(actual_addr))
    {
        printf("PPU: %x %x\n", addr, data);
        return ppu_reg_access(memmap, addr, data, access_type);
    }
    else if(check_is_controller_reg(actual_addr))
    {
        return 0;
        //return controller_access_funcs[access_type](memmap, actual_addr, data);
    }
    else if(check_is_prg_rom(actual_addr))
    {
        return prg_access_funcs[access_type](memmap, actual_addr-CPU_MEM_PRG_LOCATION0, data);
    }
    else
    {
        return cpu_access_funcs[access_type](memmap, actual_addr, data);
    }
}


static uint16_t ppu_memory_address_demirror(uint16_t addr)
{
    if(addr < PPU_MEM_NAME_TABLE_MIRRORS_OFFSET)
        return addr;
    else if(addr < PPU_MEM_NAME_TABLE1_MIRROR_OFFSET)
        return PPU_MEM_NAME_TABLE0_OFFSET + (addr % PPU_MEM_NAME_TABLE_MIRRORS_REPEAT);
    else if(addr < PPU_MEM_NAME_TABLE2_MIRROR_OFFSET)
        return PPU_MEM_NAME_TABLE1_OFFSET + (addr % PPU_MEM_NAME_TABLE_MIRRORS_REPEAT);
    else if(addr < PPU_MEM_NAME_TABLE3_MIRROR_OFFSET)
        return PPU_MEM_NAME_TABLE2_OFFSET + (addr % PPU_MEM_NAME_TABLE_MIRRORS_REPEAT);
    else if(addr < PPU_MEM_NAME_TABLE3_MIRROR_OFFSET + 0x2FF) /* Last one only goes to 3EFF */
        return PPU_MEM_NAME_TABLE3_OFFSET + (addr % 0x2FF);
    else if(addr < PPU_MEM_PALETTE_RAM_MIRRORS_OFFSET)
        return addr;
    else if(addr < PPU_MEM_SIZE)
        return PPU_MEM_PALETTE_RAM_OFFSET + (addr % PPU_MEM_PALETTE_RAM_MIRRORS_REPEAT);
    else
        return addr;
}


static uint16_t ppu_memory_read_byte(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    return (memmap->ppu_memory[addr]); 
}

static uint16_t ppu_memory_read_word(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    return ppu_memory_read_byte(memmap, addr, 0) + (ppu_memory_read_byte(memmap, addr + 1, 0) << 8);
}

static uint16_t ppu_memory_write_byte(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    memmap->ppu_memory[addr] = (uint8_t)data;
    return 0;
}

static uint16_t ppu_memory_write_word(nes_mem_td *memmap, uint16_t addr, uint16_t data) 
{
    ppu_memory_write_byte(memmap, addr, data & 0xFF);
    ppu_memory_write_byte(memmap, addr + 1, data >> 8);
    return 0;
}

static uint16_t (*ppu_access_funcs[ACCESS_FUNC_MAX])(nes_mem_td *memmap, uint16_t addr, uint16_t data) = 
{
    ppu_memory_write_word,
    ppu_memory_write_byte,
    ppu_memory_read_word,
    ppu_memory_read_byte
};

uint16_t ppu_memory_access(nes_mem_td *memmap, uint16_t addr, uint16_t data, uint8_t access_type)
{
    uint16_t actual_addr = ppu_memory_address_demirror(addr);

    //printf("addr:%x actual addr:%x\n", addr, actual_addr);

    return ppu_access_funcs[access_type](memmap, actual_addr, data);
}
