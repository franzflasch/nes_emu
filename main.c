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

    uint32_t cpu_clocks = 0;
    
    uint32_t ppu_clock_index = 0;

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
    for(i=0x8000;i<0xBFFF;i++)
    {
        printf("addr: 0x%x val: 0x%x ptr: 0x%p\n", i, *nes_mem.cpu_mem_map.mem_virt[i], nes_mem.cpu_mem_map.mem_virt[i]);
    }

    for(i=0xC000;i<0xFFFF;i++)
    {
        printf("addr: 0x%x val: 0x%x ptr: 0x%p\n", i, *nes_mem.cpu_mem_map.mem_virt[i], nes_mem.cpu_mem_map.mem_virt[i]);
    }

    /* do some test cycles */
    for(i=0;i<30000;i++)
    //for(;;)
    {
        cpu_clocks = nes_cpu_run(&nes_cpu);
        //if( i>0 &&  ((i%100) == 0)) nes_cpu_interrupt(&nes_cpu);

        /* the ppu runs at a  3 times higher clock rate than the cpu
           so we need to give the ppu some clocks here to catchup */
        for(ppu_clock_index=0;ppu_clock_index<(3*cpu_clocks);ppu_clock_index++)
            nes_ppu_run(&nes_ppu);

        nes_ppu_dump_regs(&nes_ppu);
    }

    /* Nametable 0 contents */
    for(i=0x2000;i<0x23c0;i++)
    {
        printf("%x\n", *nes_ppu.memmap->ppu_mem_map.mem_virt[i]);
    }

    // *nes_cpu.memmap->cpu_mem_map.mem_virt[0x2000] = 0x42;
    // *nes_cpu.memmap->cpu_mem_map.mem_virt[0x2001] = 0x43;
    // printf("ppuctrl: %x %x\n", nes_ppu.regs->ctrl, *nes_cpu.memmap->cpu_mem_map.mem_virt[0x2000]);
    // printf("ppumask: %x %x\n", nes_ppu.regs->mask, *nes_cpu.memmap->cpu_mem_map.mem_virt[0x2001]);

    /* Testresults are stored at 0x2 and 0x3 according to doc of nestest rom
     * if both registers are 0 everything should be fine
     */
    printf("test results: %x %x\n", *nes_mem.cpu_mem_map.mem_virt[0x2], *nes_mem.cpu_mem_map.mem_virt[0x3]);

    // return 0;
}


#if 0
#include <allegro5/allegro.h>

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
