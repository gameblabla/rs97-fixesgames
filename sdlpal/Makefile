TARGET = sdlpal.dge
DEBUG=0

ifeq ($(DEBUG),0)
  CROSS_COMPILE=mipsel-linux-
endif

CC = /opt/rs97-toolchain/bin/$(CROSS_COMPILE)gcc
CXX = /opt/rs97-toolchain/bin/$(CROSS_COMPILE)g++
STRIP = /opt/rs97-toolchain/bin/$(CROSS_COMPILE)strip

SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CFLAGS  := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS    := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)

ADPLUG_FILES = adplug/rix.cpp adplug/player.cpp adplug/binio.cpp \
	adplug/fprovide.cpp adplug/binfile.cpp adplug/dosbox_opl.cpp \
	adplug/fmopl.c adplug/surroundopl.cpp adplug/emuopl.cpp

LIBMAD_FILES = libmad/bit.c libmad/decoder.c libmad/fixed.c libmad/frame.c \
	libmad/huffman.c libmad/layer12.c libmad/layer3.c libmad/music_mad.c \
	libmad/stream.c libmad/synth.c libmad/timer.c

FILES = rixplay.cpp text.c font.c itemmenu.c scene.c palcommon.c script.c \
	util.c play.c getopt.c input.c uibattle.c game.c magicmenu.c map.c \
	ending.c uigame.c rngplay.c ui.c global.c main.c fight.c \
	video.c palette.c sound.c res.c battle.c yj1.c

FILES += $(ADPLUG_FILES)
FILES += $(LIBMAD_FILES)

CFLAGS = -g -Wall -O2 -fno-strict-aliasing $(SDL_CFLAGS)
LDFLAGS = -lstdc++ -lm $(SDL_LIBS)

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) $(FILES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
