#include <stdio.h>
#include <memory.h>

int main(int argc, char *argv[])
{
	nes_memmap_t nes_mem;

	nes_memmap_init(&nes_mem);


	/* Do some first tests */
	*nes_mem.mem_virt[0x0000] = 0x42;
	printf("addr: 0x%x val: 0x%x\n", 0x0000, *nes_mem.mem_virt[0x0000]);
	printf("addr: 0x%x val: 0x%x\n", 0x0800, *nes_mem.mem_virt[0x0800]);
	printf("addr: 0x%x val: 0x%x\n", 0x1000, *nes_mem.mem_virt[0x1000]);
	printf("addr: 0x%x val: 0x%x\n", 0x1800, *nes_mem.mem_virt[0x1800]);

	*nes_mem.mem_virt[0x2000] = 0xAB;
	printf("addr: 0x%x val: 0x%x\n", 0x2000, *nes_mem.mem_virt[0x2000]);
	printf("addr: 0x%x val: 0x%x\n", 0x2008, *nes_mem.mem_virt[0x2008]);
	printf("addr: 0x%x val: 0x%x\n", 0x2010, *nes_mem.mem_virt[0x2010]);
	printf("addr: 0x%x val: 0x%x\n", 0x2018, *nes_mem.mem_virt[0x2018]);

    printf("Hello, World!\n");
    return 0;
}
