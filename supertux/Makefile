
# Type 'make GCW0=1' to build for GCW-Zero
GCW0 = 1
APP_NAME = supertux

# compiler
ifdef GCW0
    CC = /opt/rs97-toolchain/bin/mipsel-linux-g++
else
    CC = g++
endif 

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CFLAGS  := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS    := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)

CXXDEFS = -DGP2X -DRES320X240 -DNOOPENGL -DHAVE_SOUND -I../libmikmod-3.1.21.1/include/
CXXFLAGS = $(CXXDEFS) -Wall -O2 -std=gnu++03 $(SDL_CFLAGS)
CXXLIBS = -s -lz -lSDL -lSDL_mixer -lSDL_gfx -lSDL_image

ifdef GCW0
    CXXFLAGS += -mips32
endif

# source files

OBJ =	src/badguy.o \
	src/bitmask.o \
	src/button.o \
	src/collision.o \
	src/configfile.o \
	src/intro.o \
	src/gameloop.o \
	src/globals.o \
	src/high_scores.o \
	src/level.o \
	src/leveleditor.o \
	src/lispreader.o \
	src/menu.o \
	src/particlesystem.o \
	src/physic.o \
	src/player.o \
	src/scene.o \
	src/screen.o \
	src/setup.o \
	src/special.o \
	src/supertux.o \
	src/text.o \
	src/texture.o \
	src/timer.o \
	src/title.o \
	src/type.o \
	src/world.o \
	src/worldmap.o \
	src/tile.o \
	src/mousecursor.o \
	src/resources.o \
	src/gameobjs.o \
	src/sprite.o \
	src/sprite_manager.o \
	src/music_manager.o \
	src/musicref.o \
	src/sound.o

all : $(APP_NAME)

$(APP_NAME) : $(OBJ)
	$(CC) $^ $(CXXLIBS) -o $@

%.o : %.cpp
	$(CC) -c $(CXXFLAGS) $< -o $@

clean :
	rm -rf src/*.o $(APP_NAME)
