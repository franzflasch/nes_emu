#include <nes.h>
#include <ppu.h>
#include <stdio.h>
#include <string.h>

#define debug_print(fmt, ...) \
            do { if (DEBUG_PPU) printf(fmt, __VA_ARGS__); } while (0)

#define PPU_CTRL_REG_GEN_NMI           (1 << 7)
#define PPU_CTRL_SPRITE_SIZE           (1 << 5)
#define PPU_CTRL_BG_PATTERN_TABLE_ADDR (1 << 4)

#define PPU_STATUS_REG_VBLANK  (1 << 7)
#define PPU_STATUS_SPRITE0_HIT (1 << 6)

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
    uint16_t pattern_table_load_addr = 0;
    uint16_t name_table_load_addr = 0;

    /* FIXME: This is just for testing!! sprite 0 hit has to be implemented properly! */
    if((nes_ppu->current_scan_line == 1) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->regs->status |= PPU_STATUS_SPRITE0_HIT;
    }
    else if((nes_ppu->current_scan_line == 241) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->regs->status |= PPU_STATUS_REG_VBLANK;

        if(nes_ppu->regs->ctrl & PPU_CTRL_REG_GEN_NMI)
            ppu_ret_status |= PPU_STATUS_NMI;
    }
    else if((nes_ppu->current_scan_line == 261) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->regs->status &= ~PPU_STATUS_REG_VBLANK;
        nes_ppu->regs->status &= ~PPU_STATUS_SPRITE0_HIT;
    }
    else if((nes_ppu->current_scan_line == 240) && (nes_ppu->current_pixel >= 257) && (nes_ppu->current_pixel <= 320))
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

    /* Register read and write handling */
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
            debug_print("PPU WRITE ACCESS!: %x data: %x\n", nes_ppu->memmap->last_reg_accessed, *nes_ppu->memmap->cpu_mem_map.mem_virt[nes_ppu->memmap->last_reg_accessed]);

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
            debug_print("PPU READ ACCESS!: %x data: %x\n", nes_ppu->memmap->last_reg_accessed, *nes_ppu->memmap->cpu_mem_map.mem_virt[nes_ppu->memmap->last_reg_accessed]);
            if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_STATUS_REGISTER)
            {
                nes_ppu->regs->addr = 0;
                nes_ppu->regs->status &= ~PPU_STATUS_REG_VBLANK;
                nes_ppu->curr_ppu_data_address = 0;
            }
        }

        nes_ppu->memmap->last_reg_accessed = 0;
        nes_ppu->memmap->last_reg_read_write = 0;
    }

    /* actual pixel generation begins here */
    if(nes_ppu->regs->ctrl & PPU_CTRL_SPRITE_SIZE)
    {
        printf("Spritesize of 8x16 currently not supported\n");
        while(1);
    }

    if((nes_ppu->current_scan_line < 240) && (nes_ppu->current_pixel < 256))
    {
        name_table_load_addr = PPU_MEM_NAME_TABLE0_OFFSET;

        /* calculate tile row */
        name_table_load_addr += (nes_ppu->current_pixel/8) + (0x20 * (nes_ppu->current_scan_line/8));

        pattern_table_load_addr = (nes_ppu->regs->ctrl & PPU_CTRL_BG_PATTERN_TABLE_ADDR) ? PPU_MEM_PATTERN_TABLE1_OFFSET : PPU_MEM_PATTERN_TABLE0_OFFSET;
        pattern_table_load_addr += (*nes_ppu->memmap->ppu_mem_map.mem_virt[name_table_load_addr] << 4) + (nes_ppu->current_scan_line%8);

        nes_ppu->screen_bitmap[nes_ppu->current_pixel][nes_ppu->current_scan_line] = (*nes_ppu->memmap->ppu_mem_map.mem_virt[pattern_table_load_addr] & (1 << (7-(nes_ppu->current_pixel%8)))) ? 1 : 0;
        // printf("ppu_pixel: nt addr:%x nt val:%x pt addr:%x pt val:%x tile_col:%d:%d tile_row:%d col:%d row:%d\n",
        //         name_table_load_addr,
        //         *nes_ppu->memmap->ppu_mem_map.mem_virt[name_table_load_addr],
        //         pattern_table_load_addr,
        //         *nes_ppu->memmap->ppu_mem_map.mem_virt[pattern_table_load_addr],
        //         nes_ppu->current_pixel%8, (*nes_ppu->memmap->ppu_mem_map.mem_virt[pattern_table_load_addr] & (1 << (7-(nes_ppu->current_pixel%8)))) ? 1 : 0,
        //         nes_ppu->current_scan_line%8,
        //         nes_ppu->current_pixel,
        //         nes_ppu->current_scan_line);
    }

    return ppu_ret_status;
}

void nes_ppu_dump_regs(nes_ppu_t *nes_ppu)
{
    debug_print("ctrl: %x mask: %x status: %x oamaddr: %x oamdata: %x scroll: %x addr: %x data: %x oamdma: %x pixel: %d scanline %d\n", 
            nes_ppu->regs->ctrl, nes_ppu->regs->mask, nes_ppu->regs->status, nes_ppu->regs->oamaddr, 
            nes_ppu->regs->oamdata, nes_ppu->regs->scroll, nes_ppu->regs->addr, nes_ppu->regs->data, *nes_ppu->memmap->cpu_mem_map.mem_virt[0x4014],
            nes_ppu->current_pixel, nes_ppu->current_scan_line);
}
