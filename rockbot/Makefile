TARGET 		= rockbot.dge

CROSS_COMPILE ?= /opt/rs97-toolchain/bin/mipsel-linux-
CC          := $(CROSS_COMPILE)gcc
STRIP       := $(CROSS_COMPILE)strip

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CFLAGS  := $(shell /opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config --cflags)
SDL_LIBS    := -lSDL

CFLAGS   	= -G0 -Wall -O3 -DDINGUX -DHANDHELD -g -I. $(SDL_CFLAGS)
CXXFLAGS 	= $(CFLAGS) -fexceptions -fno-rtti
LDFLAGS 	= -lm -lc -lstdc++ -lc -lSDL -lSDL_mixer -lvorbisidec -logg -lSDL_ttf -lfreetype -lSDL_image -ljpeg -lpng -lz $(SDL_LIBS)

OBJS   		= main.o options/key_map.o inputlib.o timerlib.o soundlib.o file/file_io.o collision_detection.o graphicslib.o \
sceneslib.o stage_select.o graphic/option_picker.o classmap.o objects/object.o character/character.o projectilelib.o \
graphic/animation.o stage.o class_config.o scenes/dialogs.o game.o character/classplayer.o \
character/classnpc.o character/artificial_inteligence.o aux_tools/trajectory_parabola.o file/convert.o \
graphic/draw.o graphic/gfx_sin_wave.o character/movement/jump.o character/movement/inertia.o scenes/sceneshow.o \
game_mediator.o ports/psp/psp_ram.o strings_map.o file/fio_strings.o file/fio_common.o \
file/fio_scenes.o aux_tools/stringutils.o aux_tools/fps_control.o docs/game_manual.o

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
 
