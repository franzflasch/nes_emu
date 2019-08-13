#include <stdio.h>
#include <stdlib.h>

/* NES specific */
#include <memory.h>
#include <ppu.h>
#include <cpu.h>
#include <cartridge.h>

int main(int argc, char *argv[])
{
    static nes_memmap_t nes_mem;

    static nes_ppu_t nes_ppu;
    static nes_cpu_t nes_cpu;
    static nes_cartridge_t nes_cart;


    if(argc != 2)
    {
        printf("Please specify rom file\n");
        exit(-1);
    }


    /* init memory map */
    nes_memmap_init(&nes_mem);

    /* init ppu */
    nes_ppu_init(&nes_ppu, &nes_mem);

    // /* Do some mem tests */
    // printf("addr: 0x%x val: 0x%x\n", 0x2003, *nes_mem.cpu_mem_map.mem_virt[0x2003]);
    // nes_ppu_write_oam_data(&nes_mem, 0x42);
    // nes_ppu_write_oam_data(&nes_mem, 0x42);
    // nes_ppu_write_oam_data(&nes_mem, 0x42);
    // nes_ppu_write_oam_data(&nes_mem, 0x42);
    // nes_ppu_write_oam_data(&nes_mem, 0x42);
    // printf("addr: 0x%x val: 0x%x\n", 0x2003, *nes_mem.cpu_mem_map.mem_virt[0x2003]);

    // printf("addr: 0x%x val: 0x%x\n", 0x00, nes_mem.ppu_mem_map.oam_data[0x00]);
    // printf("addr: 0x%x val: 0x%x\n", 0x01, nes_mem.ppu_mem_map.oam_data[0x01]);
    // printf("addr: 0x%x val: 0x%x\n", 0x02, nes_mem.ppu_mem_map.oam_data[0x02]);
    // printf("addr: 0x%x val: 0x%x\n", 0x03, nes_mem.ppu_mem_map.oam_data[0x03]);
    // printf("addr: 0x%x val: 0x%x\n", 0x04, nes_mem.ppu_mem_map.oam_data[0x04]);
    // printf("addr: 0x%x val: 0x%x\n", 0x05, nes_mem.ppu_mem_map.oam_data[0x05]);


    /* load rom */
    if(nes_cart_load_rom(&nes_cart, nes_mem.cpu_mem_map.mem_virt, argv[1]) != 0)
    {
        printf("ROM does not exist\n");
        exit(-2);
    }
    nes_cart_print_rom_metadata(&nes_cart);

    /* init cpu */
    nes_cpu_init(&nes_cpu, &nes_mem);

    // // /* Do some first tests */
    // // *nes_mem.mem_virt[0x0000] = 0x42;
    // // printf("addr: 0x%x val: 0x%x\n", 0x0000, *nes_mem.mem_virt[0x0000]);
    // // printf("addr: 0x%x val: 0x%x\n", 0x0800, *nes_mem.mem_virt[0x0800]);
    // // printf("addr: 0x%x val: 0x%x\n", 0x1000, *nes_mem.mem_virt[0x1000]);
    // // printf("addr: 0x%x val: 0x%x\n", 0x1800, *nes_mem.mem_virt[0x1800]);

    // // *nes_mem.mem_virt[0x2000] = 0xAB;
    // // printf("addr: 0x%x val: 0x%x\n", 0x2000, *nes_mem.mem_virt[0x2000]);
    // // printf("addr: 0x%x val: 0x%x\n", 0x2008, *nes_mem.mem_virt[0x2008]);
    // // printf("addr: 0x%x val: 0x%x\n", 0x2010, *nes_mem.mem_virt[0x2010]);
    // // printf("addr: 0x%x val: 0x%x\n", 0x2018, *nes_mem.mem_virt[0x2018]);

    int i = 0;
    for(i=0x8000;i<0x8FFF;i++)
    {
        printf("addr: 0x%x val: 0x%x ptr: 0x%p\n", i, *nes_mem.cpu_mem_map.mem_virt[i], nes_mem.cpu_mem_map.mem_virt[i]);
    }

    for(i=0xC000;i<0xCFFF;i++)
    {
        printf("addr: 0x%x val: 0x%x ptr: 0x%p\n", i, *nes_mem.cpu_mem_map.mem_virt[i], nes_mem.cpu_mem_map.mem_virt[i]);
    }

    /* do some test cycles */
    for(i=0;i<9000;i++)
    {
        nes_cpu_run(&nes_cpu);
        //if( i>0 &&  ((i%100) == 0)) nes_cpu_interrupt(&nes_cpu);
    }

    /* Testresults are stored at 0x2 and 0x3 according to doc of nestest rom
     * if both registers are 0 everything should be fine
     */
    printf("test results: %x %x\n", *nes_mem.cpu_mem_map.mem_virt[0x2], *nes_mem.cpu_mem_map.mem_virt[0x3]);

    // return 0;
}
