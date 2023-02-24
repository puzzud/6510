cmake_minimum_required(VERSION 3.10.0)

project(cmos6510)

set(CMAKE_CXX_FLAGS_DEBUG "-fPIC -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "-fPIC -O3")
set(CMAKE_CXX_FLAGS -pthread)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -Wl,--no-whole-archive")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES}")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
else()
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ ${CMAKE_CXX_STANDARD_LIBRARIES}")
endif()

# Options
OPTION(EMBEDDED_ROMS "Build CBM ROMs into binary" OFF)
if(EMBEDDED_ROMS)
    add_definitions(-DEMBEDDED_ROMS)
endif()

OPTION(SDL "Build SDL version" OFF)
if(SDL)
    add_definitions(-DSDL)
endif()

OPTION(EMSCRIPTEN "Build web version" OFF)
if(EMSCRIPTEN)
    add_definitions(-D__EMSCRIPTEN__)
endif()

if(EMSCRIPTEN)
    set(CMAKE_C_COMPILER "emcc")
    set(CMAKE_CXX_COMPILER "emcc")
    set(USE_FLAGS "-s USE_SDL=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif(EMSCRIPTEN)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED_ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Sources
set(CORE_SRC
    A64Core/BasicRom.cpp
    A64Core/Bus.cpp
    A64Core/CBM64Main.cpp
    A64Core/CharRom.cpp
    A64Core/Device.cpp
    A64Core/Rom.cpp
    A64Core/KernalRom.cpp
    A64Core/MOS6510.cpp
    A64Core/MOS6510Debug.cpp
    A64Core/MOS6526_CIA1.cpp
    A64Core/MOS6526_CIA2.cpp
    A64Core/MOS6569.cpp
    A64Core/MOS6581.cpp
    A64Core/Ram.cpp
    A64Core/Util.cpp
    A64Core/Watcher.cpp
)

set(CL_SRC
)

set(CL_MAIN_SRC
    main.cpp
)

set(SDL_SRC
    sdl/SDLHWController.cpp
    sdl/SDLHWScreen.cpp
    sdl/SDLMainLoop.cpp
)

set(SDL_MAIN_SRC
    sdl/main.cpp
)

# Default: CL
set(SRC
    ${CL_SRC}
)

set(MAIN_SRC
    ${CL_MAIN_SRC}
)

# SDL
if(SDL)
    set(SRC
        ${SDL_SRC}
    )

    set(MAIN_SRC
        ${SDL_MAIN_SRC}
    )
endif(SDL)

# Static libraries
add_library(A64Core STATIC
    ${CORE_SRC}
    ${ROMS_COMMAND}
)

# ROMs
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

# Executable
add_executable(${PROJECT_NAME}
    ${CORE_SRC}
    ${SRC}
    ${MAIN_SRC}
    ${ROMS_COMMAND}
)

# Info
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

# Includes & libraries
set(PROJECT_INCLUDE_DIRECTORIES
)

find_package(Threads REQUIRED)

set(CORE_TARGET_LIBRARIES
    Threads::Threads
)

set(PROJECT_TARGET_LIBRARIES
    ${CORE_TARGET_LIBRARIES}
    A64Core
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

# Linking
#target_link_options(
#    ${PROJECT_NAME}
#    PUBLIC "--preload-file roms"
#)

include_directories(${PROJECT_INCLUDE_DIRECTORIES})

target_link_libraries(A64Core PRIVATE ${CORE_TARGET_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_TARGET_LIBRARIES} ${PROJECT_TARGET_LIBRARIES})

