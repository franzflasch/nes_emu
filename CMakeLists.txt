cmake_minimum_required (VERSION 2.8)
project (nes_emu)

set(CMAKE_C_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS "-Wall -Werror")

#version number
set (nes_emu_VERSION_MAJOR 0)
set (nes_emu_VERSION_MINOR 0)

include_directories(nes)
#indicate the entry point for the executable
add_executable (nes_emu main.c nes/memory.c nes/cpu.c nes/cartridge.c nes/ppu.c)
target_link_libraries(nes_emu -lallegro -lallegro_primitives)

install (TARGETS nes_emu DESTINATION bin)
