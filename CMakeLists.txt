cmake_minimum_required (VERSION 2.8)
project (nes_emu)

#version number
set (nes_emu_VERSION_MAJOR 0)
set (nes_emu_VERSION_MINOR 0)

include_directories(nes)
#indicate the entry point for the executable
add_executable (nes_emu main.c nes/memory.c)

install (TARGETS nes_emu DESTINATION bin)
