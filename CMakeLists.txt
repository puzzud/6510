cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#set(CMAKE_CXX_FLAGS -pthread)

OPTION(EMBEDDED_ROMS "Build CBM ROMs into binary" OFF)
if(EMBEDDED_ROMS)
    add_definitions(-DEMBEDDED_ROMS)
endif()

OPTION(SDL "Build SDL version" OFF)
if(SDL)
    add_definitions(-DSDL)
endif()

project(cmos6510)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED_ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CORE_SRC
    A64Core/BasicRom.cpp
    A64Core/Bus.cpp
    A64Core/CBM64Main.cpp
    A64Core/CharRom.cpp
    A64Core/Device.cpp
    A64Core/KernalRom.cpp
    A64Core/MOS6510.cpp
    A64Core/MOS6510Debug.cpp
    A64Core/MOS6526_CIA1.cpp
    A64Core/MOS6526_CIA2.cpp
    A64Core/MOS6569.cpp
    A64Core/MOS6581.cpp
    A64Core/Ram.cpp
    A64Core/Util.cpp 
)

set(CL_SRC
    main.cpp
)

set(SDL_SRC
    sdl/SDLHWController.cpp
    sdl/SDLHWScreen.cpp
    sdl/SDLMainLoop.cpp
    sdl/main.cpp
)

# Default: CL
set(SRC
    ${CORE_SRC}
    ${CL_SRC}
)

# SDL
if(SDL)
    set(SRC
        ${CORE_SRC}
        ${SDL_SRC}
    )
endif(SDL)

add_custom_command(OUTPUT copy_roms
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/roms ${CMAKE_CURRENT_BINARY_DIR}/roms
)

add_custom_command(OUTPUT roms.o
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/roms && ld -r -b binary -o ${CMAKE_CURRENT_BINARY_DIR}/roms.o KERNAL.ROM BASIC.ROM CHAR.ROM
    COMMAND objcopy --rename-section .data=.rodata,alloc,load,readonly,data,contents ${CMAKE_CURRENT_BINARY_DIR}/roms.o ${CMAKE_CURRENT_BINARY_DIR}/roms.o)

if(EMBEDDED_ROMS)
    set(ROMS_COMMAND roms.o)
else()
    set(ROMS_COMMAND copy_roms)
endif(EMBEDDED_ROMS)

add_executable(
    ${PROJECT_NAME}
    ${SRC}
    ${ROMS_COMMAND}
)

message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

set(PROJECT_INCLUDE_DIRECTORIES
)

find_package(Threads REQUIRED)

set(PROJECT_TARGET_LIBRARIES
    Threads::Threads
)

# SDL
if(SDL)
    include(FindPkgConfig)
    pkg_search_module(SDL2 REQUIRED sdl2)

    set(PROJECT_INCLUDE_DIRECTORIES
        ${PROJECT_INCLUDE_DIRECTORIES}
        ${SDL2_INCLUDE_DIRS}
    )

    set(PROJECT_TARGET_LIBRARIES
        ${PROJECT_TARGET_LIBRARIES}
        ${SDL2_LIBRARIES}
    )
endif(SDL)

#target_compile_options(
#    ${PROJECT_NAME}
#    PUBLIC "-pthread"
#)

include_directories(${PROJECT_INCLUDE_DIRECTORIES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_TARGET_LIBRARIES})
