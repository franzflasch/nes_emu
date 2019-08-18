#include <cartridge.h>
#include <stdio.h>
#include <stdlib.h>

#define CPU_PRG_LOCATION0 0x8000
#define CPU_PRG_LOCATION1 0xC000

#define PPU_PATTERN_TABLE0_LOCATION 0x0000
#define PPU_PATTERN_TABLE1_LOCATION 0x1000

static void memory_write_byte_cpu(nes_cartridge_t *nes_cart, uint16_t addr, uint8_t data) 
{
    *nes_cart->memmap->cpu_mem_map.mem_virt[addr] = data;
}

static void memory_write_byte_ppu(nes_cartridge_t *nes_cart, uint16_t addr, uint8_t data) 
{
    *nes_cart->memmap->ppu_mem_map.mem_virt[addr] = data;
}

int nes_cart_load_rom(nes_cartridge_t *nes_cart, nes_memmap_t *memmap, char *rom)
{
    FILE *fp;
    int i = 0;
    uint8_t tmp = 0;

    /* At first set the memory interface */
    nes_cart->memmap = memmap;

    fp = fopen(rom,"r");
    if(fp == NULL) 
    {
        return ERR_FILE_NOT_EXIST;
    }

    if(fread(nes_cart->header, sizeof(uint8_t), 16, fp) != 16) 
    {
        return ERR_NES_FILE_HEADER_READ_FAILED;
    } 
    else 
    {
        nes_cart->prg_rom_size = 16 * 1024 * nes_cart->header[4];
        nes_cart->chr_rom_size = 8  * 1024 * nes_cart->header[5];
        nes_cart->prg_ram_size = 8  * 1024 * nes_cart->header[8];
        //if(nes_cart->chr_rom_size == 0) { nes_cart->chr_rom_size = 8 * 1024; }
        //if(nes_cart->prg_ram_size == 0) { nes_cart->prg_ram_size = 8 * 1024; }
    }

    // nes_cart->prg_rom = (uint8_t *)malloc((size_t)nes_cart->prg_rom_size);
    // nes_cart->chr_rom = (uint8_t *)malloc((size_t)nes_cart->chr_rom_size);

    // if(nes_cart->prg_rom == NULL || nes_cart->chr_rom == NULL) 
    // {
    //     return ERR_MEMORY_ALLOCATE_FAILED;
    // }

    for(i=0;i<nes_cart->prg_rom_size;i++)
    {
        if(fread(&tmp, sizeof(uint8_t), 1, fp) != 1) 
            printf("Err fread\n");

        memory_write_byte_cpu(nes_cart, CPU_PRG_LOCATION0+i, tmp);
        memory_write_byte_cpu(nes_cart, CPU_PRG_LOCATION1+i, tmp);
    }

    // if(fread(nes_cart->prg_rom, sizeof(uint8_t), (size_t)nes_cart->prg_rom_size, fp) != (size_t)nes_cart->prg_rom_size) 
    // {
    //     return ERR_PRG_ROM_LOAD_FAILED;
    // }

    // nes_cart->chr_rom = (uint8_t *)malloc((size_t)nes_cart->chr_rom_size);
    // if(fread(nes_cart->chr_rom, sizeof(uint8_t), (size_t)nes_cart->chr_rom_size, fp) != (size_t)nes_cart->chr_rom_size) 
    // {
    //     return ERR_CHR_ROM_LOAD_FAILED;
    // }

    for(i=0;i<nes_cart->chr_rom_size;i++)
    {
        if(fread(&tmp, sizeof(uint8_t), 1, fp) != 1) 
        {
            printf("Err fread chr rom\n");
            while(1);
        }

        memory_write_byte_ppu(nes_cart, PPU_PATTERN_TABLE0_LOCATION+i, tmp);
        memory_write_byte_ppu(nes_cart, PPU_PATTERN_TABLE1_LOCATION+i, tmp);
    }

    // for(i=0;i<nes_cart->chr_rom_size;i++)
    // {
    //     printf("%02x ", nes_cart->chr_rom[i]);
    //     if((i%8)==7) printf("\n");
    // }
    //exit(0);


    fclose(fp);

    return 0;
}

void nes_cart_print_rom_metadata(nes_cartridge_t *nes_cart) 
{
    const uint16_t MAPPER_TYPE_NROM = 0x1A;

    printf("==============================================\n");
    printf("ROM Metadata:\n");
    printf("Sinature: %c%c%c %s\n", nes_cart->header[0], nes_cart->header[1], nes_cart->header[2], nes_cart->header[3] == MAPPER_TYPE_NROM ? "NROM" : "Unknown");
    printf("Type: %d\n", nes_cart->header[5]);
    printf("PRG ROM Size: %d B\n", nes_cart->prg_rom_size);
    printf("CHR ROM Size: %d B\n", nes_cart->chr_rom_size);
    printf("PRG RAM Size: %d B\n", nes_cart->prg_ram_size);
    printf("==============================================\n\n");
}

// void nes_cart_exit(nes_cartridge_t *nes_cart) 
// {
//     free(nes_cart->prg_rom);
//     free(nes_cart->chr_rom);
//     nes_cart->prg_rom = NULL;
//     nes_cart->chr_rom = NULL;
// }
