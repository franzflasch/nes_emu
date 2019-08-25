#include <nes.h>
#include <ppu.h>
#include <stdio.h>
#include <string.h>

#define debug_print(fmt, ...) \
            do { if (DEBUG_PPU) printf(fmt, __VA_ARGS__); } while (0)

#define PPU_CTRL_REG_GEN_NMI           (0x1 << 7)
#define PPU_CTRL_SPRITE_SIZE           (0x1 << 5)
#define PPU_CTRL_BG_PATTERN_TABLE_ADDR (0x1 << 4)
#define PPU_CTRL_BASE_NAME_TABLE_ADDR  (0x3 << 0)

#define PPU_MASK_SHOW_BG  (1 << 3)

#define PPU_STATUS_REG_VBLANK  (1 << 7)
#define PPU_STATUS_SPRITE0_HIT (1 << 6)

ppu_color_pallete_t color_pallete_2C02[] = {
    { 84, 84, 84}, {  0, 30,116}, {  8, 16,144}, { 48,  0,136}, { 68,  0,100}, { 92,  0, 48}, { 84,  4,  0}, { 60, 24,  0}, { 32, 42,  0}, {  8, 58,  0}, {  0, 64,  0}, {  0, 60,  0}, {  0, 50, 60}, {  0,  0,  0}, {  0,  0,  0}, {  0,  0,  0},
    {152,150,152}, {  8, 76,196}, { 48, 50,236}, { 92, 30,228}, {136, 20,176}, {160, 20,100}, {152, 34, 32}, {120, 60,  0}, { 84, 90,  0}, { 40,114,  0}, {  8,124,  0}, {  0,118, 40}, {  0,102,120}, {  0,  0,  0}, {  0,  0,  0}, {  0,  0,  0},
    {236,238,236}, { 76,154,236}, {120,124,236}, {176, 98,236}, {228, 84,236}, {236, 88,180}, {236,106,100}, {212,136, 32}, {160,170,  0}, {116,196,  0}, { 76,208, 32}, { 56,204,108}, { 56,180,204}, { 60, 60, 60}, {  0,  0,  0}, {  0,  0,  0},
    {236,238,236}, {168,204,236}, {188,188,236}, {212,178,236}, {236,174,236}, {236,174,212}, {236,180,176}, {228,196,144}, {204,210,120}, {180,222,120}, {168,226,144}, {152,226,180}, {160,214,228}, {160,162,160}, {  0,  0,  0}, {  0,  0,  0}
};

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
    uint8_t tile_low_bit, tile_high_bit = 0;
    uint8_t pattern_low_val, pattern_high_val = 0;
    uint8_t current_tile_pixel_col, current_tile_pixel_row = 0;
    uint16_t attribute_table_load_addr = 0;
    uint8_t attribute_bits = 0;
    uint8_t attribute_bit_quadrant = 0;
    uint16_t color_pallete_address = 0;
    uint8_t color_pallete_index = 0;
    ppu_color_pallete_t color_pallete_value = { 0 };

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

    /* FIXME: This is just for testing!! sprite 0 hit has to be implemented properly! */
    if((nes_ppu->current_scan_line == 30) && (nes_ppu->current_pixel == 90))
    {
        nes_ppu->regs->status |= PPU_STATUS_SPRITE0_HIT;
    }
    else if((nes_ppu->current_scan_line == 241) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->regs->status |= PPU_STATUS_REG_VBLANK;

        if(nes_ppu->regs->ctrl & PPU_CTRL_REG_GEN_NMI)
        {
            ppu_ret_status |= PPU_STATUS_NMI;
        }
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
            ppu_ret_status |= PPU_STATUS_OAM_ACCESS;
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

            if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_CTRL_REGISTER)
            {
                /* Reset vram nametable address */
                nes_ppu->internal_t &= ~(0x3 << 10);
                nes_ppu->internal_t |= (nes_ppu->regs->ctrl & PPU_CTRL_BASE_NAME_TABLE_ADDR) << 10;
            }
            else if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_ADDR_REGISTER)
            {
                if(!nes_ppu->internal_w)
                {
                    nes_ppu->internal_t &= ~(0xff << 8);
                    nes_ppu->internal_t |= (nes_ppu->regs->addr << 8);
                    /* Clear bit 14 and 15 */
                    nes_ppu->internal_t &= ~(0x3 << 14);
                }
                else
                {
                    nes_ppu->internal_t &= ~(0xff);
                    nes_ppu->internal_t |= (nes_ppu->regs->addr);
                    nes_ppu->internal_v = nes_ppu->internal_t;
                }
                nes_ppu->internal_w = (nes_ppu->internal_w + 1) % 2;
            }
            else if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_DATA_REGISTER)
            {
                if(nes_ppu->internal_v >= PPU_MEM_VIRT_SIZE)
                {
                    printf("ILLEGAL ADDRESS: %x\n", nes_ppu->internal_v);
                    while(1);
                }

                *nes_ppu->memmap->ppu_mem_map.mem_virt[nes_ppu->internal_v] = *nes_ppu->memmap->cpu_mem_map.mem_virt[nes_ppu->memmap->last_reg_accessed];

                /* check address increment bit */
                nes_ppu->internal_v += (nes_ppu->regs->ctrl & 0x04) ? 32 : 1;
            }
            else if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_SCROLL_REGISTER)
            {
                if(!nes_ppu->internal_w)
                {
                    /* set scroll x */
                    nes_ppu->internal_t &= ~0x1f;
                    nes_ppu->internal_t |= ((nes_ppu->regs->scroll >> 3) & 0x1f);
                    nes_ppu->internal_x = (nes_ppu->regs->scroll & 0x7);
                    debug_print("SCROLLx %d!\n", ((nes_ppu->internal_t & 0x1f) << 3) | (nes_ppu->internal_x & 0x7) );
                }
                else
                {
                    nes_ppu->internal_t &= ~(0x1f << 5);
                    nes_ppu->internal_t &= ~(0x7 << 12);
                    nes_ppu->internal_t |= ((nes_ppu->regs->scroll >> 3) << 5);
                    nes_ppu->internal_t |= ((nes_ppu->regs->scroll & 0x7) << 12);
                    //printf("SCROLLy %d!\n", nes_ppu->scroll_offset_y);
                }
                nes_ppu->internal_w = (nes_ppu->internal_w + 1) % 2;
            }
        }
        /* read */
        else if(nes_ppu->memmap->last_reg_read_write == REG_ACCESS_READ)
        {
            printf("PPU READ ACCESS!: %x data: %x scanline: %d\n", nes_ppu->memmap->last_reg_accessed, *nes_ppu->memmap->cpu_mem_map.mem_virt[nes_ppu->memmap->last_reg_accessed], nes_ppu->current_scan_line);
            if(nes_ppu->memmap->last_reg_accessed == CPU_MEM_PPU_STATUS_REGISTER)
            {
                nes_ppu->regs->addr = 0;
                nes_ppu->regs->status &= ~PPU_STATUS_REG_VBLANK;
                //nes_ppu->curr_ppu_data_address = 0;
                nes_ppu->internal_w = 0;

                //nes_ppu->scroll_access_cycle = 0;
                //nes_ppu->scroll_offset_x = 0;
                //nes_ppu->scroll_offset_y = 0;
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

    //static uint16_t last_scroll_x_val = 0;
    uint16_t current_pixel_x = nes_ppu->current_pixel;// + nes_ppu->scroll_offset_x;
    uint16_t current_pixel_y = nes_ppu->current_scan_line;// + nes_ppu->scroll_offset_y;
    uint16_t scroll_offset_x = ((nes_ppu->internal_t & 0x1f) << 3) | (nes_ppu->internal_x & 0x7);
    uint16_t current_pixel_with_scroll_offset_x = (current_pixel_x + scroll_offset_x) % 256;
    uint16_t nt_offset = (nes_ppu->internal_t & (0x3 << 10)) ? 0x400 : 0;
    //uint16_t nt_offset = ((nes_ppu->regs->ctrl & PPU_CTRL_BASE_NAME_TABLE_ADDR) && 0x1 ) ? 0x400 : 0;

    if((nes_ppu->current_scan_line < 240) && (nes_ppu->current_pixel < 256) && (nes_ppu->regs->mask & PPU_MASK_SHOW_BG))
    {
        if((current_pixel_x+scroll_offset_x) > 255)
        {
            if(nt_offset == 0x00)
                nt_offset = 0x400;
            else 
                nt_offset = 0x00;
        }
        
        /* Update v reg */
        nes_ppu->internal_v &= ~(0x3 << 10);
        nes_ppu->internal_v |= nt_offset;
        nes_ppu->internal_v &= ~0x001F;
        nes_ppu->internal_v &= ~0x03E0;
        nes_ppu->internal_v |= (((current_pixel_y/8) << 5) & 0x3E0) + ((current_pixel_with_scroll_offset_x/8) & 0x1F);

        /* calculate tile row */
        name_table_load_addr = 0x2000 | (nes_ppu->internal_v & 0x0FFF);
        //name_table_load_addr = ( PPU_MEM_NAME_TABLE0_OFFSET + nt_offset + (0x20 * ((current_pixel_y)/8) + (current_pixel_with_scroll_offset_x/8) ));

        // if(current_pixel_x == 0)
        //     printf("v-reg:%x %x\n", 0x2000 | (nes_ppu->internal_v & 0x0FFF), name_table_load_addr);

        current_tile_pixel_row = current_pixel_y%8;

        /* get pattern from table */
        pattern_table_load_addr = (nes_ppu->regs->ctrl & PPU_CTRL_BG_PATTERN_TABLE_ADDR) ? PPU_MEM_PATTERN_TABLE1_OFFSET : PPU_MEM_PATTERN_TABLE0_OFFSET;
        pattern_table_load_addr += (*nes_ppu->memmap->ppu_mem_map.mem_virt[name_table_load_addr] << 4) + current_tile_pixel_row;

        /* get low and high pattern bits */
        pattern_low_val = *nes_ppu->memmap->ppu_mem_map.mem_virt[pattern_table_load_addr];
        pattern_high_val = *nes_ppu->memmap->ppu_mem_map.mem_virt[pattern_table_load_addr+8];

        /* now calc current pixel */
        current_tile_pixel_col = 7-((current_pixel_with_scroll_offset_x)%8);

        tile_low_bit = (pattern_low_val >> current_tile_pixel_col) & 1;
        tile_high_bit = (pattern_high_val >> current_tile_pixel_col) & 1;

        /* Get attribute bits */
        attribute_bit_quadrant = (((current_pixel_y%32)/16) << 1 ) | (((current_pixel_with_scroll_offset_x)%32)/16);
        attribute_bit_quadrant *= 2;
        attribute_table_load_addr = 0x23C0 | (nes_ppu->internal_v & 0x0C00) | ((nes_ppu->internal_v >> 4) & 0x38) | ((nes_ppu->internal_v >> 2) & 0x07);
        //attribute_table_load_addr = (PPU_MEM_ATTRIBUTE_TABLE0_OFFSET + nt_offset + (0x8 * (current_pixel_y/32)) + ((current_pixel_with_scroll_offset_x)/32));
        attribute_bits = (*nes_ppu->memmap->ppu_mem_map.mem_virt[attribute_table_load_addr] >> attribute_bit_quadrant) & 0x3;

        color_pallete_address = PPU_MEM_PALETTE_RAM_OFFSET + (attribute_bits << 2) + ((tile_high_bit<<1) | tile_low_bit);
        /* use backdrop color (0x3F00) */
        if((color_pallete_address == 0x3F04) || (color_pallete_address == 0x3F08) || (color_pallete_address == 0x3F0C)) color_pallete_address = 0x3F00;
        //color_pallete_address += (tile_high_bit<<1) | tile_low_bit;

        color_pallete_index = *nes_ppu->memmap->ppu_mem_map.mem_virt[color_pallete_address];
        color_pallete_value = color_pallete_2C02[color_pallete_index];

        //if(current_pixel_x == 4*8 && current_pixel_y == 0*8)
        // if(name_table_load_addr >= PPU_MEM_NAME_TABLE1_OFFSET && current_pixel_x == 4*8 && current_pixel_y == 0*8)
        // {
        //     printf("nt-addr:%x quadrant: %d attr-bits:%x attr-val:%x att-load-addr:%x color-pallete-addr:%x color-pallete-val:%x scrollx:%d scrolly:%d\n", 
        //             name_table_load_addr,
        //             attribute_bit_quadrant, 
        //             attribute_bits,
        //             *nes_ppu->memmap->ppu_mem_map.mem_virt[attribute_table_load_addr], 
        //             attribute_table_load_addr, 
        //             color_pallete_address, 
        //             color_pallete_index, 
        //             nes_ppu->scroll_offset_x,
        //             nes_ppu->scroll_offset_y);
        //     // int tmp = 0;
        //     // for(int i=0x3F00;i<=0x3F1F;i++)
        //     // {
        //     //     if(((i%4) == 0) && (i > 0x3F00)) tmp++;
        //     //     printf("color palette: %x %x\n", i, *nes_ppu->memmap->ppu_mem_map.mem_virt[i]);
        //     // }
        //     // for(int i=PPU_MEM_NAME_TABLE1_OFFSET;i<=PPU_MEM_ATTRIBUTE_TABLE1_OFFSET;i++)
        //     // {
        //     //     printf("nt: %x %x\n", i, *nes_ppu->memmap->ppu_mem_map.mem_virt[i]);
        //     // }
        // }

        // (void)attribute_table_load_addr;
        // (void)attribute_bits;
        // (void)color_pallete_address;
        // if(color_pallete_address > sizeof(color_pallete_2C02)/sizeof(color_pallete_2C02[0]))
        // {
        //     printf("error color pallete out of bounds\n");
        //     while(1);
        // }
        // (void)tile_low_bit;
        // (void)tile_high_bit;
        nes_ppu->screen_bitmap[current_pixel_x + (256*current_pixel_y)] = (0xFF << 24) | 
                                                                          (color_pallete_value.r << 16) | 
                                                                          (color_pallete_value.g << 8) | 
                                                                          (color_pallete_value.b); //tile_low_bit ? 0xFFFFFFFF : 0;
        //nes_ppu->screen_bitmap[current_pixel_x + (256*current_pixel_y)] = tile_high_bit ? 0xFFFFFFFF : 0;
        //printf("ppu_pixel: %x\n", color_pallete_address);
        // printf("ppu_pixel: nt addr:%x nt val:%x pt addr:%x pt val:%x tile_col:%d:%d tile_row:%d col:%d row:%d\n",
        //         name_table_load_addr,
        //         *nes_ppu->memmap->ppu_mem_map.mem_virt[name_table_load_addr],
        //         pattern_table_load_addr,
        //         *nes_ppu->memmap->ppu_mem_map.mem_virt[pattern_table_load_addr],
        //         current_pixel_x%8, (*nes_ppu->memmap->ppu_mem_map.mem_virt[pattern_table_load_addr] & (1 << (7-(current_pixel_x%8)))) ? 1 : 0,
        //         current_pixel_y%8,
        //         current_pixel_x,
        //         current_pixel_y);
    }

    // //if(scroll_offset_x < last_scroll_x_val)
    // if(((nes_ppu->regs->ctrl & PPU_CTRL_BASE_NAME_TABLE_ADDR) && 0x1 ) && nes_ppu->current_pixel == 0)
    // {
    //     printf("WHAT!? %x %d %d %x %d %d %x %x %x\n", nes_ppu->regs->mask, scroll_offset_x, last_scroll_x_val, nt_offset, nes_ppu->current_scan_line, nes_ppu->current_pixel, nes_ppu->regs->mask, nes_ppu->regs->status, nes_ppu->regs->ctrl);
    //     //scroll_offset_x = last_scroll_x_val;
    //     //while(1);
    // }
    // // else 
    //     last_scroll_x_val = scroll_offset_x;

    return ppu_ret_status;
}

void nes_ppu_dump_regs(nes_ppu_t *nes_ppu)
{
    debug_print("ctrl: %x mask: %x status: %x oamaddr: %x oamdata: %x scroll: %x addr: %x data: %x oamdma: %x pixel: %d scanline %d\n", 
            nes_ppu->regs->ctrl, nes_ppu->regs->mask, nes_ppu->regs->status, nes_ppu->regs->oamaddr, 
            nes_ppu->regs->oamdata, nes_ppu->regs->scroll, nes_ppu->regs->addr, nes_ppu->regs->data, *nes_ppu->memmap->cpu_mem_map.mem_virt[0x4014],
            nes_ppu->current_pixel, nes_ppu->current_scan_line);
}
