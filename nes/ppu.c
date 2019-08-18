#include <ppu.h>
#include <stdio.h>
#include <string.h>

// /* CPU <-> PPU Shared Registers */
// #define CPU_MEM_PPU_REG_OAM_ADDR 0x2003

// void nes_ppu_write_oam_data(nes_memmap_t *memmap, uint8_t val)
// {
//     uint8_t addr = *memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REG_OAM_ADDR];
//     memmap->ppu_mem_map.oam_data[addr] = val;

//     /* increment addr */
//     *memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REG_OAM_ADDR] = (*memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REG_OAM_ADDR]+1)%PPU_MEM_OAMD_SIZE;
// }

// uint8_t nes_ppu_read_oam_data(nes_memmap_t *memmap)
// {
//     uint8_t addr = *memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REG_OAM_ADDR];
//     return memmap->ppu_mem_map.oam_data[addr];
// }

void nes_ppu_init(nes_ppu_t *nes_ppu, nes_memmap_t *memmap)
{   
    memset(nes_ppu, 0, sizeof(*nes_ppu));

    nes_ppu->memmap = memmap;

    /* map ppu registers into the shared CPU memory */
    nes_ppu->regs = (ppu_regs_t *) memmap->cpu_mem_map.mem_virt[CPU_MEM_PPU_REGISTER_OFFSET];
}

uint8_t nes_ppu_run(nes_ppu_t *nes_ppu)
{   

    uint8_t ppu_ret_status = 0;

    if( nes_ppu->memmap->last_reg_accessed &&
        ( 
            ( 
              (nes_ppu->memmap->last_reg_accessed >= CPU_MEM_PPU_REGISTER_OFFSET) &&
              (nes_ppu->memmap->last_reg_accessed <= CPU_MEM_PPU_REGISTER_AREA_END) 
            ) ||
            (nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_OAMDMA_REGISTER)
        ) 
      )
    {
        if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_OAMDMA_REGISTER)
        {
            printf("OAMDMA access currently not implemented!\n");
            //while(1);
        }
        else
        {
            /* Maybe some insane NES game uses mirrored memory addresses */
            nes_ppu->memmap->last_reg_accessed = (nes_ppu->memmap->last_reg_accessed % 8) + CPU_MEM_PPU_REGISTER_OFFSET;
        }

        /* write */
        if(nes_ppu->memmap->last_reg_read_write == REG_ACCESS_WRITE)
        {
            printf("PPU WRITE ACCESS!: %x data: %x\n", nes_ppu->memmap->last_reg_accessed, *nes_ppu->memmap->cpu_mem_map.mem_virt[nes_ppu->memmap->last_reg_accessed]);

            /* Update least significant bits previously written into a PPU register */
            nes_ppu->regs->status &= (~0x1F);
            nes_ppu->regs->status |= (*nes_ppu->memmap->cpu_mem_map.mem_virt[nes_ppu->memmap->last_reg_accessed] & 0x1F);

            if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_ADDR_REGISTER)
            {
                if(!nes_ppu->addr_access_cycle)
                {
                    nes_ppu->curr_ppu_data_address = nes_ppu->regs->addr << 8;
                }
                else
                {
                    nes_ppu->curr_ppu_data_address |= nes_ppu->regs->addr;
                }

                nes_ppu->addr_access_cycle = (nes_ppu->addr_access_cycle + 1) % 2;
            }
            else if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_DATA_REGISTER)
            {
                if(nes_ppu->curr_ppu_data_address >= PPU_MEM_VIRT_SIZE)
                {
                    printf("ILLEGAL ADDRESS: %x\n", nes_ppu->curr_ppu_data_address);
                    while(1);
                }

                *nes_ppu->memmap->ppu_mem_map.mem_virt[nes_ppu->curr_ppu_data_address] = *nes_ppu->memmap->cpu_mem_map.mem_virt[nes_ppu->memmap->last_reg_accessed];

                /* check address increment bit */
                nes_ppu->curr_ppu_data_address += (nes_ppu->regs->ctrl & 0x04) ? 32 : 1;
            }
        }
        /* read */
        else if(nes_ppu->memmap->last_reg_read_write == REG_ACCESS_READ)
        {
            printf("PPU READ ACCESS!: %x data: %x\n", nes_ppu->memmap->last_reg_accessed, *nes_ppu->memmap->cpu_mem_map.mem_virt[nes_ppu->memmap->last_reg_accessed]);
            if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_STATUS_REGISTER)
            {
                nes_ppu->regs->addr = 0;
                nes_ppu->regs->status &= ~(1 << 7);
                nes_ppu->curr_ppu_data_address = 0;
            }
        }

        nes_ppu->memmap->last_reg_accessed = 0;
        nes_ppu->memmap->last_reg_read_write = 0;
    }


    /* FIXME: This is just for testing!! sprite 0 hit has to be implemented properly! */
    if((nes_ppu->current_scan_line == 1) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->regs->status |= (1 << 6);
    }

    if((nes_ppu->current_scan_line == 241) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->regs->status |= (1 << 7);

        if(nes_ppu->regs->ctrl & (1 << 7))
            ppu_ret_status |= PPU_STATUS_NMI;
    }

    if((nes_ppu->current_scan_line == 261) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->regs->status &= ~(1 << 7);
        nes_ppu->regs->status &= ~(1 << 6);
    }

    if((nes_ppu->current_scan_line == 240) && (nes_ppu->current_pixel >= 257) && (nes_ppu->current_pixel <= 320))
    {
        nes_ppu->regs->oamaddr = 0;
    }

    nes_ppu->current_pixel++;
    if(nes_ppu->current_pixel >= 340)
    {
        nes_ppu->current_pixel = 0;
        nes_ppu->current_scan_line++;
        if(nes_ppu->current_scan_line >= 262)
        {
            nes_ppu->current_scan_line = 0;
            ppu_ret_status |= PPU_STATUS_FRAME_READY;
        }
    }

    return ppu_ret_status;
}

void nes_ppu_dump_regs(nes_ppu_t *nes_ppu)
{
    printf("ctrl: %x mask: %x status: %x oamaddr: %x oamdata: %x scroll: %x addr: %x data: %x oamdma: %x pixel: %d scanline %d\n", 
            nes_ppu->regs->ctrl, nes_ppu->regs->mask, nes_ppu->regs->status, nes_ppu->regs->oamaddr, 
            nes_ppu->regs->oamdata, nes_ppu->regs->scroll, nes_ppu->regs->addr, nes_ppu->regs->data, *nes_ppu->memmap->cpu_mem_map.mem_virt[0x4014],
            nes_ppu->current_pixel, nes_ppu->current_scan_line);
}
