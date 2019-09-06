#include <stdio.h>
#include <stdlib.h>

/* NES specific */
#include <nes.h>
#include <ppu.h>
#include <cpu.h>
#include <cartridge.h>
#include <controller.h>

#define debug_print(fmt, ...) \
            do { if (DEBUG_MAIN) printf(fmt, __VA_ARGS__); } while (0)


uint8_t sprite_0_hit_debug = 1;

void die (const char * format, ...)
{
    va_list vargs;
    va_start (vargs, format);
    vfprintf (stderr, format, vargs);
    va_end (vargs);
    exit (1);
}

#include <SDL2/SDL.h>

#define FPS 60
#define FPS_UPDATE_TIME_MS (1000/FPS)

/* Query a button's state.
   Returns 1 if button #b is pressed. */
uint8_t nes_key_state(uint8_t b)
{
    const Uint8* keyboard;
    SDL_PumpEvents();
    keyboard = SDL_GetKeyboardState(NULL);

    switch (b)
    {
        case 0: // On / Off
            return 1;
        case 1: // A
            return keyboard[SDL_SCANCODE_A] ? 1 : 0;
        case 2: // B
            return keyboard[SDL_SCANCODE_S] ? 1 : 0;
        case 3: // SELECT
            return keyboard[SDL_SCANCODE_C] ? 1 : 0;
        case 4: // START
            return keyboard[SDL_SCANCODE_RETURN] ? 1 : 0;
        case 5: // UP
            return keyboard[SDL_SCANCODE_UP] ? 1 : 0;
        case 6: // DOWN
            return keyboard[SDL_SCANCODE_DOWN] ? 1 : 0;
        case 7: // LEFT
            return keyboard[SDL_SCANCODE_LEFT] ? 1 : 0;
        case 8: // RIGHT
            return keyboard[SDL_SCANCODE_RIGHT] ? 1 : 0;
        default:
            return 1;
    }
}

uint8_t nes_key_state_ctrl2(uint8_t b)
{
    switch (b)
    {
        case 0: // On / Off
            return 1;
        case 1: // A
            return 0;
        case 2: // B
            return 0;
        case 3: // SELECT
            return 0;
        case 4: // START
            return 0;
        case 5: // UP
            return 0;
        case 6: // DOWN
            return 0;
        case 7: // LEFT
            return 0;
        case 8: // RIGHT
            return 0;
        default:
            return 1;
    }
}

int main(int argc, char *argv[])
{
    //int i = 0;
    //static nes_memmap_t nes_mem;
    static nes_ppu_t nes_ppu;
    static nes_cpu_t nes_cpu;
    static nes_cartridge_t nes_cart;

    static nes_mem_td nes_memory = { 0 };

    uint32_t cpu_clocks = 0;
    uint32_t ppu_clock_index = 0;
    uint8_t ppu_status = 0;

    if(argc != 2)
    {
        die("Please specify rom file\n");
    }

    /* init memory map */
    //nes_memmap_init(&nes_mem);

    /* init cartridge */
    nes_cart_init(&nes_cart, &nes_memory);

    /* load rom */
    if(nes_cart_load_rom(&nes_cart, argv[1]) != 0)
    {
        die("ROM does not exist\n");
    }
    nes_cart_print_rom_metadata(&nes_cart);

    /* Set nametable mirroring after loading the cartridge */
    //nes_ppu_memmap_set_nt_mirror(&nes_mem.ppu_mem_map);

    /* init cpu */
    nes_cpu_init(&nes_cpu, &nes_memory);
    nes_cpu_reset(&nes_cpu);

    /* init ppu */
    nes_ppu_init(&nes_ppu, &nes_memory);




    unsigned int lastTime = 0, currentTime;
    //Uint32 pixels[256*240] = {0};

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        die("Failed to initialise SDL\n");
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("nes_emu",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          256*5,
                                          240*5,
                                          SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        die("Could not create a window: %s", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        die("Could not create a renderer: %s", SDL_GetError());
    }

    SDL_Texture * texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 256, 240);

    while (1)
    {
        // Get the next event
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                break;
            }
        }

        for(;;)
        {
            cpu_clocks = 0;
            if(ppu_status & PPU_STATUS_NMI)
                cpu_clocks += nes_cpu_nmi(&nes_cpu);
            cpu_clocks += nes_cpu_run(&nes_cpu);

            //controller_run(&nes_mem);

            // ppu_start:

            /* the ppu runs at a 3 times higher clock rate than the cpu
            so we need to give the ppu some clocks here to catchup */
            ppu_status = 0;
            for(ppu_clock_index=0;ppu_clock_index<(3*cpu_clocks);ppu_clock_index++)
            {
                ppu_status |= nes_ppu_run(&nes_ppu, nes_cpu.num_cycles);
            }

            // if(ppu_status & PPU_STATUS_OAM_ACCESS)
            // {
            //     cpu_clocks = 514;
            //     goto ppu_start;
            // }

            nes_ppu_dump_regs(&nes_ppu);

            if(ppu_status & PPU_STATUS_FRAME_READY) break;
        }

        const Uint8* keyboard;
        SDL_PumpEvents();
        keyboard = SDL_GetKeyboardState(NULL);

        if (keyboard[SDL_SCANCODE_P]) {
            printf("SPRITE0 HIT is on.\n");
            sprite_0_hit_debug = 1;
        }

        if (keyboard[SDL_SCANCODE_O]) {
            printf("SPRITE0 HIT is off.\n");
            sprite_0_hit_debug = 0;
        }


        // for(int i=0x0000;i<0x0FFF;i++)
        // {
        //     printf("Pattern Table 0: %x %x\n", i, (uint8_t)ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }

        // for(int i=0x1000;i<0x1FFF;i++)
        // {
        //     printf("Pattern Table 1: %x %x\n", i, (uint8_t)ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }

        // /* Nametable 0 contents */
        // for(int i=0x2000;i<0x23FF;i++)
        // {
        //     printf("Nametable 0: %x %x\n", i, (uint8_t)ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }

        // /* Nametable 1 contents */
        // for(int i=0x2400;i<0x27FF;i++)
        // {
        //     printf("Nametable 1: %x %x\n", i, (uint8_t)ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }

        // for(int i=0x3F00;i<=0x3F1F;i++)
        // {
        //     printf("PALLETE: %x %x\n", i, ppu_memory_access(&nes_memory, i, 0, ACCESS_READ_BYTE));
        // }

        // for(int i=0;i<256;i++)
        // {
        //     printf("OAM: %d %x\n", i, nes_memory.oam_memory[i]);
        // }

        SDL_UpdateTexture(texture, NULL, nes_ppu.bg_bitmap, 256 * sizeof(Uint32));

        // Randomly change the colour
        // Uint8 red = rand() % 255;
        // Uint8 green = rand() % 255;
        // Uint8 blue = rand() % 255;

        // Fill the screen with the colour
        //SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        while ((currentTime = SDL_GetTicks()) < (lastTime + FPS_UPDATE_TIME_MS));// printf("fast enough\n");
        lastTime = currentTime;
        SDL_RenderPresent(renderer);

        //memset(nes_ppu.sprite_bitmap, 0, sizeof(nes_ppu.sprite_bitmap));
    }

    // Tidy up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
