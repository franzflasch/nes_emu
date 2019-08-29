#include <cartridge.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// static void memory_write_byte_cpu(nes_cartridge_t *nes_cart, uint16_t addr, uint8_t data) 
// {
//     *nes_cart->memmap->cpu_mem_map.mem_virt[addr] = data;
// }

// static void memory_write_byte_ppu(nes_cartridge_t *nes_cart, uint16_t addr, uint8_t data) 
// {
//     *nes_cart->memmap->ppu_mem_map.mem_virt[addr] = data;
// }

int nes_cart_load_rom(nes_cartridge_t *nes_cart, char *rom)
{
    FILE *fp;
    int i = 0;
    uint8_t tmp = 0;

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
    }

    for(i=0;i<nes_cart->prg_rom_size;i++)
    {
        if(fread(&tmp, sizeof(uint8_t), 1, fp) != 1) 
            printf("Err fread\n");

        //memory_write_byte_cpu(nes_cart, CPU_MEM_PRG_LOCATION0+i, tmp);
        //memory_write_byte_cpu(nes_cart, CPU_MEM_PRG_LOCATION1+i, tmp);

        cpu_memory_access(nes_cart->nes_mem, CPU_MEM_PRG_LOCATION0+i, tmp, ACCESS_WRITE_BYTE);
        //cpu_memory_access(nes_cart->nes_mem, CPU_MEM_PRG_LOCATION1+i, tmp, ACCESS_WRITE_BYTE);
    }

    for(i=0;i<nes_cart->chr_rom_size;i++)
    {
        if(fread(&tmp, sizeof(uint8_t), 1, fp) != 1) 
        {
            printf("Err fread chr rom\n");
            while(1);
        }

        //memory_write_byte_ppu(nes_cart, PPU_MEM_PATTERN_TABLE0_OFFSET+i, tmp);
        //memory_write_byte_ppu(nes_cart, PPU_MEM_PATTERN_TABLE1_OFFSET+i, tmp);

        ppu_memory_access(nes_cart->nes_mem, PPU_MEM_PATTERN_TABLE0_OFFSET+i, tmp, ACCESS_WRITE_BYTE);
        //ppu_memory_access(nes_cart->nes_mem, PPU_MEM_PATTERN_TABLE1_OFFSET+i, tmp, ACCESS_WRITE_BYTE);
    }

    //memmap->ppu_mem_map.mirroring = nes_cart->header[6] & 0x1;

    // for(i=0;i<nes_cart->chr_rom_size;i++)
    // {
    //     printf("%02x ", nes_cart->chr_rom[i]);
    //     if((i%8)==7) printf("\n");
    // }
    //exit(0);


    fclose(fp);

    return 0;
}

void nes_cart_init(nes_cartridge_t *nes_cart, nes_mem_td *nes_mem)
{
    memset(nes_cart, 0, sizeof(*nes_cart));
    nes_cart->nes_mem = nes_mem;
}


void nes_cart_print_rom_metadata(nes_cartridge_t *nes_cart) 
{
    uint8_t mapper = 0;

    mapper = (nes_cart->header[7] & 0xF0) | ((nes_cart->header[6] & 0xFF) >> 4);

    printf("==============================================\n");
    printf("ROM Metadata:\n");
    printf("Sinature: %c%c%c\n", nes_cart->header[0], nes_cart->header[1], nes_cart->header[2]);
    printf("Flags 6: %d\n", nes_cart->header[6]);
    printf("Mirroring: %s\n", (nes_cart->header[6] & (1<<0)) ? "vertical" : "horizontal" );
    printf("Flags 7: %d\n", nes_cart->header[7]);
    printf("Mapper %x\n", mapper);
    printf("PRG ROM Size: %d B\n", nes_cart->prg_rom_size);
    printf("CHR ROM Size: %d B\n", nes_cart->chr_rom_size);
    printf("PRG RAM Size: %d B\n", nes_cart->prg_ram_size);
    printf("==============================================\n\n");

    if(mapper != 0)
    {
        printf("Mapper %x is not supported\n", mapper);
        //while(1);
    }
}
