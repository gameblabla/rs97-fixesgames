# Temper makefile
# Gilead Kutnick - Exophase

# Global definitions

TC_PREFIX = /home/exophase/pandora-dev
PREFIX    = $(TC_PREFIX)/arm-2011.03
CC        = $(PREFIX)/bin/arm-none-linux-gnueabi-gcc
AS        = $(PREFIX)/bin/arm-none-linux-gnueabi-gcc
STRIP     = $(PREFIX)/bin/arm-none-linux-gnueabi-strip

OBJS      = main.o cpu.o memory.o irq.o io.o video.o timer.o psg.o disasm.o    \
            screen.o event.o audio.o menu.o debug.o cd.o adpcm.o               \
            arcade_card.o bin_cue.o cpu_arm_compatible.o cpu_arm_fast.o        \
            netplay.o                                                          \
            pandora_main.o pandora_screen.o pandora_menu.o                     \
            pandora_event.o SDL_audio.o
BIN       ?= temper

VPATH     += .. ../arm ../SDL
CFLAGS    += -DPANDORA_BUILD -Wall -O3 -march=armv7-a -mtune=cortex-a8
INCLUDES  = -I$(PREFIX)/usr/include `$(PREFIX)/usr/bin/sdl-config --cflags`
LIBS      = `$(PREFIX)/usr/bin/sdl-config --libs` -lvorbisidec -lbz2

# Compilation:

.SUFFIXES: .c .S

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(AS) -c -o $@ $<

all:	$(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(BIN)  
	$(STRIP) $(BIN)
	cp temper pnd
	$(TC_PREFIX)/sdk_utils/pnd_make.sh -d ./pnd -x ./pnd/PXML.xml -p $(BIN).pnd  \
	 -i ./pnd/temper_icon.png
 
clean:
	rm -f *.o $(BIN) 

