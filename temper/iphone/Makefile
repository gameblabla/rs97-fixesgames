# Temper makefile
# Gilead Kutnick - Exophase

# Global definitions

PREFIX    = /Developer/Platforms/iPhoneOS.platform/Developer
CC        = $(PREFIX)/usr/bin/arm-apple-darwin9-gcc

OBJS      = main.o cpu.o memory.o irq.o io.o video.o timer.o psg.o cd.o       \
            adpcm.o arcade_card.o bin_cue.o disasm.o menu.o screen.o audio.o  \
            event.o debug.o iphone.o cpu_arm_compatible.o cpu_arm_fast.o      \
            video_arm_asm.o temper4iphone/main.o                              \
            temper4iphone/Classes/OptionsController.o                         \
            temper4iphone/Classes/SaveStatesController.o                      \
            temper4iphone/Classes/SOApplication.o                             \
            temper4iphone/Classes/RecentController.o                          \
            temper4iphone/NowPlayingController.o                              \
            temper4iphone/Classes/RootViewController.o                        \
            temper4iphone/Classes/ShoutOutAppDelegate.o                       \
            temper4iphone/Classes/TabBar.o 

BIN       ?= temper4iphone_

# Platform specific definitions 

VPATH      += ..
VPATH      += ../arm
CFLAGS     += -Wall -DIPHONE_BUILD
CFLAGS     += -O3 -std=c99 -funsigned-char -fno-common                        \
	            -fno-builtin

INCLUDES   =  -F$(PREFIX)/SDKs/iPhoneOS2.0.sdk/System/Library/Frameworks      \
              -F$(PREFIX)/SDKs/iPhoneOS2.0.sdk/System/Library/PrivateFrameworks  \
              -I./temper4iphone/ -I./temper4iphone/Classes/ -I./temper4iphone/Classes/AdMob/                            \
              -I$(PREFIX)/SDKs/iPhoneOS2.0.sdk/usr/lib/gcc/arm-apple-darwin9/4.0.1/include   \
              -isysroot $(PREFIX)/SDKs/iPhoneOS2.0.sdk                        \
              -L./temper4iphone/Classes/AdMob/                                              \
              -L$(PREFIX)/SDKs/iPhoneOS2.0.sdk/usr/lib
LIBS       =  -lobjc -lpthread -framework CoreFoundation                      \
              -framework Foundation -framework UIKit -framework QuartzCore    \
              -framework CoreGraphics -framework CoreSurface                  \
              -framework CoreLocation -framework AudioToolbox                 \
              -framework GraphicsServices -lAdMobDevice -lpthread -lbz2       \
              -ltremolo

# Compilation:

.SUFFIXES: .c .S .s .m

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.o: %.m
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.o: %.s
	${CC} -DIPHONE_BUILD -c -o $@ $<

%.o: %.S
	${CC} -DIPHONE_BUILD -c -o $@ $<

all:	$(OBJS)
	$(CC) ${INCLUDES} $(OBJS) $(LIBS) -o $(BIN)  

#	$(STRIP) $(BIN)

clean:
	rm -f *.o *.u *.z $(BIN) 

