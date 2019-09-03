#include <stdio.h>
#include <string.h>

#include <nes.h>
#include <ppu.h>

#define debug_print(fmt, ...) \
            do { if (DEBUG_PPU) printf(fmt, __VA_ARGS__); } while (0)

#define PPU_CTRL_REG_GEN_NMI           (0x1 << 7)
#define PPU_CTRL_SPRITE_SIZE           (0x1 << 5)
#define PPU_CTRL_BG_PATTERN_TABLE_ADDR (0x1 << 4)
#define PPU_CTRL_SP_PATTERN_TABLE_ADDR (0x1 << 3)
#define PPU_CTRL_VRAM_INCREMENT        (0x1 << 2)
#define PPU_CTRL_BASE_NAME_TABLE_ADDR  (0x3 << 0)

#define PPU_MASK_SHOW_BG       (1 << 3)
#define PPU_MASK_SHOW_SPRITES  (1 << 4)

#define PPU_STATUS_REG_VBLANK  (1 << 7)
#define PPU_STATUS_SPRITE0_HIT (1 << 6)

ppu_color_pallete_t color_pallete_2C02[] = {
    { 84, 84, 84}, {  0, 30,116}, {  8, 16,144}, { 48,  0,136}, { 68,  0,100}, { 92,  0, 48}, { 84,  4,  0}, { 60, 24,  0}, { 32, 42,  0}, {  8, 58,  0}, {  0, 64,  0}, {  0, 60,  0}, {  0, 50, 60}, {  0,  0,  0}, {  0,  0,  0}, {  0,  0,  0},
    {152,150,152}, {  8, 76,196}, { 48, 50,236}, { 92, 30,228}, {136, 20,176}, {160, 20,100}, {152, 34, 32}, {120, 60,  0}, { 84, 90,  0}, { 40,114,  0}, {  8,124,  0}, {  0,118, 40}, {  0,102,120}, {  0,  0,  0}, {  0,  0,  0}, {  0,  0,  0},
    {236,238,236}, { 76,154,236}, {120,124,236}, {176, 98,236}, {228, 84,236}, {236, 88,180}, {236,106,100}, {212,136, 32}, {160,170,  0}, {116,196,  0}, { 76,208, 32}, { 56,204,108}, { 56,180,204}, { 60, 60, 60}, {  0,  0,  0}, {  0,  0,  0},
    {236,238,236}, {168,204,236}, {188,188,236}, {212,178,236}, {236,174,236}, {236,174,212}, {236,180,176}, {228,196,144}, {204,210,120}, {180,222,120}, {168,226,144}, {152,226,180}, {160,214,228}, {160,162,160}, {  0,  0,  0}, {  0,  0,  0}
};

uint16_t ppu_reg_access(nes_mem_td *memmap, uint16_t addr, uint16_t data, uint8_t access_type)
{
    uint16_t i = 0;

    if((access_type == ACCESS_READ_WORD) || (access_type == ACCESS_WRITE_WORD))
    {
        die("Invalid PPU reg access! %d\n", access_type);
    }

    if(access_type == ACCESS_WRITE_BYTE)
    {
        /* Update least significant bits previously written into a PPU register */
        memmap->ppu_regs.status &= (~0x1F);
        memmap->ppu_regs.status |= (data & 0x1F);

        if(addr == CPU_MEM_PPU_CTRL_REGISTER)
        {
            memmap->ppu_regs.ctrl = data;
            /* Reset vram nametable address */
            memmap->internal_t &= ~(0x3 << 10);
            memmap->internal_t |= (memmap->ppu_regs.ctrl & PPU_CTRL_BASE_NAME_TABLE_ADDR) << 10;
        }
        else if(addr == CPU_MEM_PPU_MASK_REGISTER)
        {
            memmap->ppu_regs.mask = data;
        }
        else if(addr == CPU_MEM_PPU_OAMADDR_REGISTER)
        {
            memmap->ppu_regs.oamaddr = data;
        }
        else if(addr == CPU_MEM_PPU_OAMDATA_REGISTER)
        {
            memmap->oam_memory[memmap->ppu_regs.oamaddr] = data;
            memmap->ppu_regs.oamaddr++;
        }
        else if(addr == CPU_MEM_PPU_ADDR_REGISTER)
        {
            memmap->ppu_regs.addr = data;

            if(!memmap->internal_w)
            {
                memmap->internal_t &= ~(0xff << 8);
                memmap->internal_t |= (memmap->ppu_regs.addr << 8);
                /* Clear bit 14 and 15 */
                memmap->internal_t &= ~(0x3 << 14);
            }
            else
            {
                memmap->internal_t &= ~(0xff);
                memmap->internal_t |= (memmap->ppu_regs.addr);
                memmap->internal_v = memmap->internal_t;
            }
            memmap->internal_w = (memmap->internal_w + 1) % 2;
        }
        else if(addr == CPU_MEM_PPU_DATA_REGISTER)
        {
            memmap->ppu_regs.data = data;
            if(memmap->internal_v >= PPU_MEM_SIZE)
            {
                die("ILLEGAL ADDRESS: %x\n", memmap->internal_v);
            }

            ppu_memory_access(memmap, memmap->internal_v, memmap->ppu_regs.data, ACCESS_WRITE_BYTE);

            /* check address increment bit */
            memmap->vram_add = (memmap->ppu_regs.ctrl & PPU_CTRL_VRAM_INCREMENT) ? 32 : 1;
        }
        else if(addr == CPU_MEM_PPU_SCROLL_REGISTER)
        {
            memmap->ppu_regs.scroll = data;
            if(!memmap->internal_w)
            {
                /* set scroll x */
                memmap->internal_t &= ~0x1f;
                memmap->internal_t |= ((memmap->ppu_regs.scroll >> 3) & 0x1f);
                memmap->internal_x = (memmap->ppu_regs.scroll & 0x7);
                debug_print("SCROLLx %d!\n", ((memmap->internal_t & 0x1f) << 3) | (memmap->internal_x & 0x7) );
            }
            else
            {
                memmap->internal_t &= ~(0x1f << 5);
                memmap->internal_t &= ~(0x7 << 12);
                memmap->internal_t |= ((memmap->ppu_regs.scroll >> 3) << 5);
                memmap->internal_t |= ((memmap->ppu_regs.scroll & 0x7) << 12);
            }
            memmap->internal_w = (memmap->internal_w + 1) % 2;
        }
        else if(addr == CPU_MEM_PPU_OAMDMA_REGISTER)
        {
            for(i=0;i<256;i++)
            {
                memmap->oam_memory[i] = (uint8_t)cpu_memory_access(memmap, ((data << 8) | i), 0, ACCESS_READ_BYTE);
            }
            //memmap->oam_memory[memmap->ppu_regs.oamaddr] = data;
        }
    }
    /* read */
    else if(access_type == ACCESS_READ_BYTE)
    {
        if(addr == CPU_MEM_PPU_STATUS_REGISTER)
        {
            uint8_t tmp_status = memmap->ppu_regs.status;
            memmap->ppu_regs.addr = 0;
            memmap->ppu_regs.status &= ~PPU_STATUS_REG_VBLANK;
            memmap->internal_w = 0;
            return tmp_status;
        }
        else if(addr == CPU_MEM_PPU_DATA_REGISTER)
        {
            uint8_t latched = memmap->vram_latch;
            uint8_t retval = 0;

            if(memmap->internal_v >= PPU_MEM_SIZE)
            {
                die("ILLEGAL ADDRESS: %x\n", memmap->internal_v);
            }

            memmap->vram_latch = (uint8_t)ppu_memory_access(memmap, memmap->internal_v, 0, ACCESS_READ_BYTE);

            if(memmap->internal_v >= PPU_MEM_PALETTE_RAM_OFFSET)
                retval = memmap->vram_latch;
            else
                retval = latched;

            memmap->vram_add = (memmap->ppu_regs.ctrl & PPU_CTRL_VRAM_INCREMENT) ? 32 : 1;
            return retval;
        }
        else if(addr == CPU_MEM_PPU_OAMDATA_REGISTER)
        {
            return memmap->oam_memory[memmap->ppu_regs.oamaddr];
        }
    }

    return 0;
}

void nes_ppu_init(nes_ppu_t *nes_ppu, nes_mem_td *nes_memory)
{
    memset(nes_ppu, 0, sizeof(*nes_ppu));

    nes_ppu->nes_memory = nes_memory;
}

uint8_t nes_ppu_run(nes_ppu_t *nes_ppu, uint32_t cpu_cycles)
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
    if((nes_ppu->current_scan_line == 30) && (nes_ppu->current_pixel == 88))
    {
        nes_ppu->nes_memory->ppu_regs.status |= PPU_STATUS_SPRITE0_HIT;
    }
    else if((nes_ppu->current_scan_line == 241) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->nes_memory->ppu_regs.status |= PPU_STATUS_REG_VBLANK;

        if(nes_ppu->nes_memory->ppu_regs.ctrl & PPU_CTRL_REG_GEN_NMI)
        {
            ppu_ret_status |= PPU_STATUS_NMI;
        }
    }
    else if((nes_ppu->current_scan_line == 261) && (nes_ppu->current_pixel == 1))
    {
        nes_ppu->nes_memory->ppu_regs.status &= ~PPU_STATUS_REG_VBLANK;
        nes_ppu->nes_memory->ppu_regs.status &= ~PPU_STATUS_SPRITE0_HIT;
        debug_print("vblank cycles: %d\n", cpu_cycles);
    }
    else if((nes_ppu->current_scan_line == 240) && (nes_ppu->current_pixel >= 257) && (nes_ppu->current_pixel <= 320))
    {
        nes_ppu->nes_memory->ppu_regs.oamaddr = 0;
    }

    /* actual pixel generation begins here */
    if(nes_ppu->nes_memory->ppu_regs.ctrl & PPU_CTRL_SPRITE_SIZE)
    {
        die("Spritesize of 8x16 currently not supported %x\n", nes_ppu->nes_memory->ppu_regs.ctrl);
    }

    uint16_t current_pixel_x = nes_ppu->current_pixel;
    uint16_t current_pixel_y = nes_ppu->current_scan_line;
    uint16_t scroll_offset_x = ((nes_ppu->nes_memory->internal_t & 0x1f) << 3) | (nes_ppu->nes_memory->internal_x & 0x7);
    uint16_t current_pixel_with_scroll_offset_x = (current_pixel_x + scroll_offset_x) % 256;
    uint16_t nt_offset = (nes_ppu->nes_memory->internal_t & (0x3 << 10)) ? 0x400 : 0;


    /* Increment vreg after $2007 access */
    nes_ppu->nes_memory->internal_v += nes_ppu->nes_memory->vram_add;
    nes_ppu->nes_memory->vram_add = 0;

    if((nes_ppu->current_scan_line < 240) && (nes_ppu->current_pixel < 256))
    {
        current_tile_pixel_row = current_pixel_y%8;
        current_tile_pixel_col = 7-((current_pixel_with_scroll_offset_x)%8);

        if(nes_ppu->nes_memory->ppu_regs.mask & PPU_MASK_SHOW_BG) 
        {
            if((current_pixel_x+scroll_offset_x) > 255)
            {
                if(nt_offset == 0x00)
                    nt_offset = 0x400;
                else 
                    nt_offset = 0x00;
            }
            
            nes_ppu->nes_memory->internal_v &= ~(0x3 << 10);
            nes_ppu->nes_memory->internal_v |= nt_offset;
            nes_ppu->nes_memory->internal_v &= ~0x001F;
            nes_ppu->nes_memory->internal_v &= ~0x03E0;
            nes_ppu->nes_memory->internal_v |= (((current_pixel_y/8) << 5) & 0x3E0) + ((current_pixel_with_scroll_offset_x/8) & 0x1F);

            /* calculate tile row */
            name_table_load_addr = 0x2000 | (nes_ppu->nes_memory->internal_v & 0x0FFF);

            /* get pattern from table */
            pattern_table_load_addr = (nes_ppu->nes_memory->ppu_regs.ctrl & PPU_CTRL_BG_PATTERN_TABLE_ADDR) ? PPU_MEM_PATTERN_TABLE1_OFFSET : PPU_MEM_PATTERN_TABLE0_OFFSET;
            pattern_table_load_addr += (ppu_memory_access(nes_ppu->nes_memory, name_table_load_addr, 0, ACCESS_READ_BYTE) << 4) + current_tile_pixel_row;

            /* get low and high pattern bits */
            pattern_low_val = (uint8_t)ppu_memory_access(nes_ppu->nes_memory, pattern_table_load_addr, 0, ACCESS_READ_BYTE);
            pattern_high_val = (uint8_t)ppu_memory_access(nes_ppu->nes_memory, pattern_table_load_addr+8, 0, ACCESS_READ_BYTE);

            tile_low_bit = (pattern_low_val >> current_tile_pixel_col) & 1;
            tile_high_bit = (pattern_high_val >> current_tile_pixel_col) & 1;

            /* Get attribute bits */
            attribute_bit_quadrant = (((current_pixel_y%32)/16) << 1 ) | (((current_pixel_with_scroll_offset_x)%32)/16);
            attribute_bit_quadrant *= 2;
            attribute_table_load_addr = 0x23C0 | (nes_ppu->nes_memory->internal_v & 0x0C00) | ((nes_ppu->nes_memory->internal_v >> 4) & 0x38) | ((nes_ppu->nes_memory->internal_v >> 2) & 0x07);
            attribute_bits = ((uint8_t)ppu_memory_access(nes_ppu->nes_memory, attribute_table_load_addr, 0, ACCESS_READ_BYTE) >> attribute_bit_quadrant) & 0x3;

            color_pallete_address = PPU_MEM_PALETTE_RAM_OFFSET + (attribute_bits << 2) + ((tile_high_bit<<1) | tile_low_bit);
            /* use backdrop color (0x3F00) */
            if((color_pallete_address == 0x3F04) || (color_pallete_address == 0x3F08) || (color_pallete_address == 0x3F0C)) color_pallete_address = 0x3F00;

            color_pallete_index = (uint8_t)ppu_memory_access(nes_ppu->nes_memory, color_pallete_address, 0, ACCESS_READ_BYTE);
            color_pallete_value = color_pallete_2C02[color_pallete_index];

            /* Draw the pixel */
            nes_ppu->bg_bitmap[current_pixel_x + (256*current_pixel_y)] = (0xFF << 24) | 
                                                                            (color_pallete_value.r << 16) | 
                                                                            (color_pallete_value.g << 8) | 
                                                                            (color_pallete_value.b);
        }

        if(nes_ppu->nes_memory->ppu_regs.mask & PPU_MASK_SHOW_SPRITES)
        {
            uint16_t i = 0;
            uint16_t x_index,y_index = 0;
            uint8_t sprite_val_low = 0;
            uint8_t sprite_val_high = 0;
            uint16_t sprite_start_pix_x = 0;
            uint16_t sprite_start_pix_y = 0;
            uint8_t flip_vertical = 0;
            uint8_t flip_horizontal = 0;
            uint8_t sprite_x_index = 0;
            uint8_t sprite_y_index = 0;

            pattern_table_load_addr = (nes_ppu->nes_memory->ppu_regs.ctrl & PPU_CTRL_SP_PATTERN_TABLE_ADDR) ? PPU_MEM_PATTERN_TABLE1_OFFSET : PPU_MEM_PATTERN_TABLE0_OFFSET;

            for(i=0;i<PPU_MEM_OAMD_SIZE;i+=4)
            {
                if(nes_ppu->current_scan_line == nes_ppu->nes_memory->oam_memory[i] && nes_ppu->current_pixel == nes_ppu->nes_memory->oam_memory[i+3])
                {
                    pattern_table_load_addr += (nes_ppu->nes_memory->oam_memory[i+1] << 4);
                    //printf("Sprite %d Load addr: %x %d %d\n", i/4, pattern_table_load_addr, current_pixel_x, current_pixel_y);
                    sprite_start_pix_x = nes_ppu->nes_memory->oam_memory[i+3];
                    sprite_start_pix_y = nes_ppu->nes_memory->oam_memory[i];

                    flip_horizontal = nes_ppu->nes_memory->oam_memory[i+2] & (1 << 7) ? 1 : 0;
                    flip_vertical = nes_ppu->nes_memory->oam_memory[i+2] & (1 << 6) ? 1 : 0;

                    for(y_index=0;y_index<8;y_index++)
                    {
                        sprite_y_index = flip_horizontal ? (7-y_index) : y_index;
                        sprite_val_low = (uint8_t)ppu_memory_access(nes_ppu->nes_memory, pattern_table_load_addr + sprite_y_index, 0, ACCESS_READ_BYTE);
                        sprite_val_high = (uint8_t)ppu_memory_access(nes_ppu->nes_memory, pattern_table_load_addr + sprite_y_index + 8, 0, ACCESS_READ_BYTE);
                        //printf("Sprite %d low: %x high: %x\n", i/4, sprite_val_low, sprite_val_high);
                        for(x_index=0;x_index<8;x_index++)
                        {
                            sprite_x_index = flip_vertical ? (1 << (x_index)) : (1 << (7-x_index));
                            if((sprite_val_low & sprite_x_index) || (sprite_val_high & sprite_x_index ))
                            {
                                nes_ppu->sprite_bitmap[(sprite_start_pix_x+x_index) + (256*(sprite_start_pix_y+y_index))] = (0xFF << 24) | 
                                                                                                                           (0xFF << 16) | 
                                                                                                                           (0xFF << 8) | 
                                                                                                                           (0xFF);
                            }
                        }
                    }

                    // for(j=0;j<16;j++)
                    // {
                    //     sprite_val = (uint8_t)ppu_memory_access(nes_ppu->nes_memory, pattern_table_load_addr + j, 0, ACCESS_READ_BYTE);
                    //     nes_ppu->screen_bitmap[nes_ppu->nes_memory->oam_memory[i+3] + (256*current_pixel_y)] = (0xFF << 24) | 
                    //                                                                     (color_pallete_value.r << 16) | 
                    //                                                                     (color_pallete_value.g << 8) | 
                    //                                                                     (color_pallete_value.b);
                    //     //printf("match: %d x:%d y:%d byte1:%x byte2:%x spr:%x\n", i, nes_ppu->nes_memory->oam_memory[i+3], nes_ppu->nes_memory->oam_memory[i], nes_ppu->nes_memory->oam_memory[i+1], nes_ppu->nes_memory->oam_memory[i+2], sprite_val);
                    // }
                }
            }
        }
    }

    return ppu_ret_status;
}

void nes_ppu_dump_regs(nes_ppu_t *nes_ppu)
{
    debug_print("ctrl: %x mask: %x status: %x oamaddr: %x oamdata: %x scroll: %x addr: %x data: %x pixel: %d scanline %d\n", 
            nes_ppu->nes_memory->ppu_regs.ctrl, nes_ppu->nes_memory->ppu_regs.mask, nes_ppu->nes_memory->ppu_regs.status, nes_ppu->nes_memory->ppu_regs.oamaddr, 
            nes_ppu->nes_memory->ppu_regs.oamdata, nes_ppu->nes_memory->ppu_regs.scroll, nes_ppu->nes_memory->ppu_regs.addr, nes_ppu->nes_memory->ppu_regs.data,
            nes_ppu->current_pixel, nes_ppu->current_scan_line);
}
