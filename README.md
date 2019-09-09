# nes_emu  
## This is a Nintendo Entertainment System - NES Emulator written in plain C.  

I wrote this emulator for fun and because I wanted to have a portable NES emulator, which I could (theoretically) even run on a microcontroller. Altough it would be very slow, as the code is definitely not performance optimized.  

- The CPU code is taken from https://github.com/blanboom/bEMU.git, with a lot of improvemens. Now it is instanceable and supports more unofficial opcodes.  
- PPU code was written entirely from scratch, but could stand some refactoring.  
- It passes some but not all NES tests from the https://github.com/christopherpow/nes-test-roms.git repo.  
- It is actually able to play some NROM games like Super Mario Bros., Ice Climber, Donkey Kong, etc.  

![Alt text](pictures/super_mario.png?raw=true "Super Mario Bros")

* FEATURES MISSING:  
    - PPU: Needs a general refactoring.  
    - PPU: Sprite priority needs to be implemented - reason for some minor graphical glitches in Super Mario Bros.  
    - PPU: Not sure if sprite0 hit is implemented properly.  
    - CPU - PPU Communication and Timing are probably not correct, as most of the blargg timing tests are failing.  
    - Currently only NROM games work, some other mappers would be great.  

## Dependencies
- libsdl2-dev
- cmake
- gcc

## Building
```bash
mkdir build
cd build
cmake ..
make
```

## Controls  
A: A
B: S
Select: C
Start: Enter
D-Pad: Arrow keys

## Porting
Porting should be pretty easy. To run the Emulator Core you only need to do this:  

```C
#include <nes.h>
#include <ppu.h>
#include <cpu.h>
#include <cartridge.h>
#include <controller.h>

/* Initialization */
static nes_ppu_t nes_ppu;
static nes_cpu_t nes_cpu;
static nes_cartridge_t nes_cart;
static nes_mem_td nes_memory = { 0 };

uint32_t cpu_clocks = 0;
uint32_t ppu_clocks = 0;
uint32_t ppu_rest_clocks = 0;
uint32_t ppu_clock_index = 0;
uint8_t ppu_status = 0;

/* init cartridge */
nes_cart_init(&nes_cart, &nes_memory);

/* load rom */
if(nes_cart_load_rom(&nes_cart, argv[1]) != 0)
{
    die("ROM does not exist\n");
}

/* init cpu */
nes_cpu_init(&nes_cpu, &nes_memory);
nes_cpu_reset(&nes_cpu);

/* init ppu */
nes_ppu_init(&nes_ppu, &nes_memory);

/* This is the main loop */
while(1)
{
    for(;;)
    {
        cpu_clocks = 0;
        if(!ppu_rest_clocks)
        {
            if(ppu_status & PPU_STATUS_NMI)
                cpu_clocks += nes_cpu_nmi(&nes_cpu);
            cpu_clocks += nes_cpu_run(&nes_cpu);
        }

        /* the ppu runs at a 3 times higher clock rate than the cpu
        so we need to give the ppu some clocks here to catchup */
        ppu_clocks = (cpu_clocks*3) + ppu_rest_clocks;
        ppu_status = 0;
        for(ppu_clock_index=0;ppu_clock_index<ppu_clocks;ppu_clock_index++)
        {
            ppu_status |= nes_ppu_run(&nes_ppu, nes_cpu.num_cycles);
            if(ppu_status & PPU_STATUS_FRAME_READY) break;
            else ppu_rest_clocks = 0;
        }

        ppu_rest_clocks = (ppu_clocks - ppu_clock_index);

        nes_ppu_dump_regs(&nes_ppu);

        if(ppu_status & PPU_STATUS_FRAME_READY) break;
    }

    /* Draw nes_ppu.screen_bitmap on the screen here */
}
```

The pixels produced by the inner loop are stored in  
```C
nes_ppu.screen_bitmap  
```

It's up to you to use some graphic library and draw those pixels onto the screen outside the for(;;) loop. The core code has no logic to limit the framerate, so FPS limit has to be done outside the for(;;) loop. See main.c as an example with libsdl2.  


## License
Licensed under GPLv3.  

Feel free to send PRs :)  
