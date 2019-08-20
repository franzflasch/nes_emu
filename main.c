#include <stdio.h>
#include <stdlib.h>

/* NES specific */
#include <nes.h>
#include <memory.h>
#include <ppu.h>
#include <cpu.h>
#include <cartridge.h>

#define debug_print(fmt, ...) \
            do { if (DEBUG_MAIN) printf(fmt, __VA_ARGS__); } while (0)

ppu_color_pallete_2C02_t color_pallete_2C02[] = {
    { 84, 84, 84}, {  0, 30,116}, {  8, 16,144}, { 48,  0,136}, { 68,  0,100}, { 92,  0, 48}, { 84,  4,  0}, { 60, 24,  0}, { 32, 42,  0}, {  8, 58,  0}, {  0, 64,  0}, {  0, 60,  0}, {  0, 50, 60}, {  0,  0,  0}, {  0,  0,  0}, {  0,  0,  0},
    {152,150,152}, {  8, 76,196}, { 48, 50,236}, { 92, 30,228}, {136, 20,176}, {160, 20,100}, {152, 34, 32}, {120, 60,  0}, { 84, 90,  0}, { 40,114,  0}, {  8,124,  0}, {  0,118, 40}, {  0,102,120}, {  0,  0,  0}, {  0,  0,  0}, {  0,  0,  0},
    {236,238,236}, { 76,154,236}, {120,124,236}, {176, 98,236}, {228, 84,236}, {236, 88,180}, {236,106,100}, {212,136, 32}, {160,170,  0}, {116,196,  0}, { 76,208, 32}, { 56,204,108}, { 56,180,204}, { 60, 60, 60}, {  0,  0,  0}, {  0,  0,  0},
    {236,238,236}, {168,204,236}, {188,188,236}, {212,178,236}, {236,174,236}, {236,174,212}, {236,180,176}, {228,196,144}, {204,210,120}, {180,222,120}, {168,226,144}, {152,226,180}, {160,214,228}, {160,162,160}, {  0,  0,  0}, {  0,  0,  0}
};

#include <allegro5/allegro.h>

int main(int argc, char *argv[])
{
    int i = 0;
    static nes_memmap_t nes_mem;
    static nes_ppu_t nes_ppu;
    static nes_cpu_t nes_cpu;
    static nes_cartridge_t nes_cart;
    uint32_t cpu_clocks = 0;
    uint32_t ppu_clock_index = 0;
    uint8_t ppu_status = 0;

    if(argc != 2)
    {
        printf("Please specify rom file\n");
        exit(-1);
    }

    /* init memory map */
    nes_memmap_init(&nes_mem);

    /* init ppu */
    nes_ppu_init(&nes_ppu, &nes_mem);


    /* load rom */
    if(nes_cart_load_rom(&nes_cart, &nes_mem, argv[1]) != 0)
    {
        printf("ROM does not exist\n");
        exit(-2);
    }
    nes_cart_print_rom_metadata(&nes_cart);

    /* init cpu */
    nes_cpu_init(&nes_cpu, &nes_mem);




    /* FIXME: TURN OFF debug prints if setting FPS to a high value otherwise won't work */
    const float FPS = 5;
    const int SCREEN_W = 256*4;
    const int SCREEN_H = 240*4;

    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *color_pallete = NULL;
    ALLEGRO_BITMAP *nes_screen = NULL;
    ALLEGRO_COLOR tmp_color = al_map_rgb(0,0,0);

    bool redraw = true;

    if(!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    color_pallete = al_create_bitmap(4, 8);
    if(!color_pallete) {
        fprintf(stderr, "failed to create color_pallete bitmap!\n");
        return -1;
    }
    al_set_target_bitmap(color_pallete);
    al_clear_to_color(al_map_rgb(0, 0, 0));

    nes_screen = al_create_bitmap(256, 240);
    if(!nes_screen) {
        fprintf(stderr, "failed to create nes_screen bitmap!\n");
        return -1;
    }
    al_set_target_bitmap(nes_screen);
    al_clear_to_color(al_map_rgb(0, 0, 0));

    al_set_target_bitmap(al_get_backbuffer(display));

    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    event_queue = al_create_event_queue();
    if(!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_bitmap(color_pallete);
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_clear_to_color(al_map_rgb(0,0,0));
    al_flip_display();
    al_start_timer(timer);



    while(1)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_TIMER) {
            /* do some test cycles */
            //for(i=0;i<100000;i++)
            for(;;)
            {
                if(ppu_status & PPU_STATUS_NMI) nes_cpu_nmi(&nes_cpu);
                cpu_clocks = nes_cpu_run(&nes_cpu);

                ppu_status = 0;

                /* ppu is initialized after ~30000 ticks */
                if((nes_cpu.num_cycles*3) > 30000)
                {
                    /* the ppu runs at a  3 times higher clock rate than the cpu
                    so we need to give the ppu some clocks here to catchup */
                    for(ppu_clock_index=0;ppu_clock_index<(3*cpu_clocks);ppu_clock_index++)
                        ppu_status |= nes_ppu_run(&nes_ppu);
                }

                nes_ppu_dump_regs(&nes_ppu);

                if(ppu_status & PPU_STATUS_FRAME_READY) break;
            }

            int pixel_col_index = 0;
            int pixel_row_index = 0;
            system("clear");
            al_set_target_bitmap(nes_screen);
            for(pixel_row_index=0;pixel_row_index<40;pixel_row_index++)
            {
                for(pixel_col_index=0;pixel_col_index<256;pixel_col_index++)
                {
                    //printf("%s",nes_ppu.screen_bitmap[pixel_col_index][pixel_row_index] ? "." : " ");
                    tmp_color.r = (double)nes_ppu.screen_bitmap[pixel_col_index][pixel_row_index];
                    al_put_pixel(pixel_col_index, pixel_row_index, tmp_color);
                }
                //printf("\n");
            }


            // /* Pattern table 0 contents */
            // for(i=0x0000;i<0x0FFF;i++)
            // {
            //     debug_print("Pattern 0: %x %x\n", i, *nes_ppu.memmap->ppu_mem_map.mem_virt[i]);
            // }

            // /* Pattern table 1 contents */
            // for(i=0x1000;i<0x1FFF;i++)
            // {
            //     debug_print("Pattern 1: %x %x\n", i, *nes_ppu.memmap->ppu_mem_map.mem_virt[i]);
            // }

            // /* Nametable 0 contents */
            // for(i=0x2000;i<0x23FF;i++)
            // {
            //     debug_print("Nametable 0: %x %x\n", i, *nes_ppu.memmap->ppu_mem_map.mem_virt[i]);
            // }

            // for(i=0x2400;i<0x27FF;i++)
            // {
            //     debug_print("Nametable 1: %x %x\n", i, *nes_ppu.memmap->ppu_mem_map.mem_virt[i]);
            // }

            // for(i=0x2800;i<0x2BFF;i++)
            // {
            //     debug_print("Nametable 2: %x %x\n", i, *nes_ppu.memmap->ppu_mem_map.mem_virt[i]);
            // }

            // for(i=0x2C00;i<0x2FFF;i++)
            // {
            //     debug_print("Nametable 3: %x %x\n", i, *nes_ppu.memmap->ppu_mem_map.mem_virt[i]);
            // }

            // for(i=0x3F00;i<0x3F1F;i++)
            // {
            //     debug_print("Color Pallete %x: %x\n", i, *nes_ppu.memmap->ppu_mem_map.mem_virt[i]);
            // }

            /* Draw color pallete */
            al_set_target_bitmap(color_pallete);
            int tmp = 0;
            for(i=0x3F00;i<=0x3F1F;i++)
            {
                if(((i%4) == 0) && (i > 0x3F00)) tmp++;
                al_put_pixel(i%4, tmp, 
                             al_map_rgb(color_pallete_2C02[*nes_ppu.memmap->ppu_mem_map.mem_virt[i]].r,
                             color_pallete_2C02[*nes_ppu.memmap->ppu_mem_map.mem_virt[i]].g,
                             color_pallete_2C02[*nes_ppu.memmap->ppu_mem_map.mem_virt[i]].b));
            }

            redraw = true;
        }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) 
        {
            break;
        }

        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_set_target_bitmap(al_get_backbuffer(display));
            al_clear_to_color(al_map_rgb(0,0,0));
            al_draw_bitmap(nes_screen, 300, 300, 0);
            al_draw_scaled_bitmap(color_pallete, 0, 0, 4, 8, 0, 0, 128, 256, 0);
            al_flip_display();
        }
    }

    al_destroy_bitmap(color_pallete);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}


#if 0
int main(int argc, char **argv)
{
    const float FPS = 60;
    const int SCREEN_W = 256;
    const int SCREEN_H = 240;
    const int BOUNCER_SIZE = 32;

    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *bouncer = NULL;
    float bouncer_x = SCREEN_W / 2.0 - BOUNCER_SIZE / 2.0;
    float bouncer_y = SCREEN_H / 2.0 - BOUNCER_SIZE / 2.0;
    float bouncer_dx = -4.0, bouncer_dy = 4.0;
    bool redraw = true;

    if(!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    bouncer = al_create_bitmap(BOUNCER_SIZE, BOUNCER_SIZE);
    if(!bouncer) {
        fprintf(stderr, "failed to create bouncer bitmap!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }
    al_set_target_bitmap(bouncer);
    al_clear_to_color(al_map_rgb(0, 0, 255));

    al_set_target_bitmap(al_get_backbuffer(display));

    event_queue = al_create_event_queue();
    if(!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_bitmap(bouncer);
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_clear_to_color(al_map_rgb(0,0,0));

    al_flip_display();

    al_start_timer(timer);

    while(1)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_TIMER) {
            if(bouncer_x < 0 || bouncer_x > SCREEN_W - BOUNCER_SIZE) {
                bouncer_dx = -bouncer_dx;
            }

            if(bouncer_y < 0 || bouncer_y > SCREEN_H - BOUNCER_SIZE) {
                bouncer_dy = -bouncer_dy;
            }

            bouncer_x += bouncer_dx;
            bouncer_y += bouncer_dy;

            al_set_target_bitmap(bouncer);
            al_put_pixel(10, 10, al_map_rgb(255,255,255));
            al_put_pixel(20, 20, al_map_rgb(255,255,255));
            al_set_target_bitmap(al_get_backbuffer(display));

            redraw = true;
        }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) 
        {
            break;
        }

        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_clear_to_color(al_map_rgb(0,0,0));
            al_draw_bitmap(bouncer, bouncer_x, bouncer_y, 0);
            al_flip_display();
        }   
    }

    al_destroy_bitmap(bouncer);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
#endif


// /* triangle.c */
// #include <curses.h>
// #include <stdlib.h>
// #define ITERMAX 10000

// int main(void)
// {
//     int maxlines;
//     //int maxcols;

//     /* initialize curses */
//     initscr();
//     cbreak();
//     noecho();
//     clear();

//     /* initialize triangle */
//     maxlines = LINES - 1;
//     //maxcols = COLS - 1;

//     start_color();
//     init_color(COLOR_RED, 800, 0, 200);
//     init_pair(1, COLOR_RED, COLOR_RED); /* create foreground / background combination */
//     attron(COLOR_PAIR(1)); /* use the above combination */
//     printw("   ");
//     attroff(COLOR_PAIR(1)); /* turn color off */


//     /* done */
//     mvaddstr(maxlines, 0, "Press any key to quit");
//     refresh();
//     getch();
//     endwin();
//     exit(0);
// }
