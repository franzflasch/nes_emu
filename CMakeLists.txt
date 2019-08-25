cmake_minimum_required (VERSION 2.8)
project (nes_emu)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS "-Wall -Werror")

#version number
set (nes_emu_VERSION_MAJOR 0)
set (nes_emu_VERSION_MINOR 0)

set(CMAKE_C_FLAGS_RELEASE "-O3")

find_package(SDL2 REQUIRED)
include_directories(nes_emu ${SDL2_INCLUDE_DIRS})

include_directories(nes)
#indicate the entry point for the executable
add_executable (nes_emu main.c nes/memory.c nes/cpu.c nes/cartridge.c nes/ppu.c nes/controller.c)
target_link_libraries(nes_emu ${SDL2_LIBRARIES})

install (TARGETS nes_emu DESTINATION bin)
