
ROOT_DIR = ..

CORE_CPP = ../A64Core/MOS6510.cpp \
    ../A64Core/BasicRom.cpp \
    ../A64Core/Bus.cpp \
    ../A64Core/CBM64Main.cpp \
    ../A64Core/CharRom.cpp \
    ../A64Core/Device.cpp \
    ../A64Core/KernalRom.cpp \
    ../A64Core/MOS6510Debug.cpp \
    ../A64Core/MOS6526_CIA1.cpp \
    ../A64Core/MOS6526_CIA2.cpp \
    ../A64Core/MOS6569.cpp \
    ../A64Core/MOS6581.cpp \
    ../A64Core/Ram.cpp \
    ../A64Core/Util.cpp 

MAIN_CPP = ../sdl/SDLHWScreen.cpp \
    ../sdl/main.cpp

SRCS = $(CORE_CPP) $(MAIN_CPP)

OBJ_DIR = ../build
OBJS = $(patsubst $(ROOT_DIR)/%,$(OBJ_DIR)/%,$(SRCS))
OBJS := $(OBJS:.cpp=.o)

ROMS = ../roms/BASIC.ROM \
    ../roms/CHAR.ROM \
    ../roms/KERNAL.ROM

ROM_COPIES = $(subst ../roms,./roms,$(ROMS))

CC = g++

CFLAGS := `sdl2-config --cflags` -Wall -g $(CFLAGS)
LFLAGS := `sdl2-config --libs` -pthread

EXECUTABLE = $(OBJ_DIR)/cmos6510_sdl

ifdef __EMSCRIPTEN__
CC := emcc
CFLAGS := -s USE_SDL=2
LFLAGS := --preload-file roms
EXECUTABLE := $(OBJ_DIR)/cmos6510.html
EMBEDDED_ROMS := 1
endif

all: $(ROM_COPIES) $(EXECUTABLE)

./roms/%: ../roms/%
	@mkdir -p $(@D)
	cp $< $@

$(OBJ_DIR)/%.o: $(ROOT_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS)

$(EXECUTABLE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(EXECUTABLE)
