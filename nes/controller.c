#include <controller.h>
#include <stdio.h>

static uint8_t prev_write;
static uint8_t p = 10;

uint8_t psg_io_read(void)
{
    // Joystick 1
    if (p++ < 9) {
        return nes_key_state(p);
    }
    return 0;
}

void psg_io_write(uint8_t data)
{
    if ((data & 1) == 0 && prev_write == 1) {
        // strobe
        p = 0;
    }
    prev_write = data & 1;
}


void controller_run(nes_memmap_t *nes_mem)
{
    /* Register read and write handling */
    if( nes_mem->last_reg_accessed &&
        (nes_mem->last_reg_accessed == CONTROLLER_PORT1_REG)
      )
    {
        if(nes_mem->last_reg_read_write == REG_ACCESS_WRITE)
        {
            if(nes_mem->last_reg_accessed == CONTROLLER_PORT1_REG)
            {
                printf("Controller 1 WRITE! %x\n", *nes_mem->cpu_mem_map.mem_virt[nes_mem->last_reg_accessed]);
                psg_io_write(*nes_mem->cpu_mem_map.mem_virt[nes_mem->last_reg_accessed]);
            }
        }
        // else if(nes_mem->last_reg_read_write == REG_ACCESS_READ)
        // {
        //     if(nes_mem->last_reg_accessed == CONTROLLER_PORT1_REG)
        //     {
        //         *nes_mem->cpu_mem_map.mem_virt[nes_mem->last_reg_accessed] = psg_io_read();
        //         printf("Controller 1 READ! %d %x\n", p, *nes_mem->cpu_mem_map.mem_virt[nes_mem->last_reg_accessed]);
        //     }
        // }

        nes_mem->last_reg_accessed = 0;
        nes_mem->last_reg_read_write = 0;
    }
}
