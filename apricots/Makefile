#PLATFORM = linux_x86
#PLATFORM = linux_x64
#PLATFORM = mingw32	# win32
#PLATFORM = cegcc	# wince (arm)
#PLATFORM = gizmondo
#PLATFORM = gp2x
#PLATFORM = wiz
#PLATFORM = caanoo
#PLATFORM = dingux
PLATFORM = gcw
#PLATFORM ?= linux_x86

### Dingoo/Dingux
ifeq ($(PLATFORM), dingux)
	CXX = mipsel-linux-g++
	STRIP = mipsel-linux-strip
	CFLAGS = -mips32 -mtune=mips32 -G0 -fomit-frame-pointer -ffunction-sections -ffast-math -fsingle-precision-constant -mbranch-likely
	INCLUDE = -I/opt/opendingux-toolchain/usr/include/SDL
	LIB = -lSDL -lpthread
endif

### GCW Zero
ifeq ($(PLATFORM), gcw)
	CXX = /opt/rs97-toolchain/bin/mipsel-linux-g++
	STRIP = /opt/rs97-toolchain/bin/mipsel-linux-strip
	CFLAGS = -mips32 -mtune=mips32 -G0 -fomit-frame-pointer -ffunction-sections -ffast-math -fsingle-precision-constant -mbranch-likely -DAP_AUDIO_SDLMIXER
	LDFLAGS = 
	INCLUDE = -I/opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/include/SDL 
	LIB = -lSDL_mixer -lSDL -lpthread
endif

### GP2X/Wiz
ifeq ($(PLATFORM), gp2x)
	CXX = arm-open2x-linux-g++
	STRIP = arm-open2x-linux-strip
	CFLAGS =
	LDFLAGS = -static # GP2X requires static linking, Wiz does not
	LDFLAGS += -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib/
	INCLUDE = -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/
	LIB = -lSDL -lpthread
endif
ifeq ($(PLATFORM), wiz)
	CXX = arm-open2x-linux-g++
	STRIP = arm-open2x-linux-strip
	CFLAGS =
	LDFLAGS += -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib/
	INCLUDE = -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/
	LIB = -lSDL -lpthread
endif

### Caanoo
ifeq ($(PLATFORM), caanoo)
	CXX = arm-gph-linux-gnueabi-g++
	STRIP = arm-gph-linux-gnueabi-strip
	CFLAGS =
	LIB = -lSDL -lpthread
endif

### x86 linux
ifeq ($(PLATFORM), linux_x86)
	CXX = g++
	STRIP = strip
	CFLAGS = -DAP_AUDIO_SDLMIXER
	INCLUDE = -I/usr/include/ -I/usr/include/SDL
	LIB = -lSDL_mixer -lSDL -lpthread
endif

### win32
ifeq ($(PLATFORM), mingw32)
	CFLAGS =
	INCLUDE = -I/usr/i486-mingw32/include
	CXX = i486-mingw32-g++
	STRIP = i486-mingw32-strip
	LIB = -lmingw32 -lSDLmain -lSDL -lpthread -mwindows
endif

### wince (arm)
ifeq ($(PLATFORM), cegcc)
	CFLAGS = -D_WIN32_IE=0x0400
	INCLUDE = -I/opt/cegcc/arm-cegcc/include/
	LDFLAGS = -static -L/opt/cegcc/arm-cegcc/lib/
	CXX = arm-cegcc-g++
	STRIP = arm-cegcc-strip
	LIB = -lSDL -lpthread -mwindows
endif

### Gizmondo
ifeq ($(PLATFORM), gizmondo)
	CFLAGS = -Wall -ffast-math -funroll-loops -mcpu=arm920 -mtune=arm920t -fstrict-aliasing -fexpensive-optimizations -falign-functions -fweb -frename-registers -fomit-frame-pointer -finline -finline-functions -fno-builtin -fno-common -mstructure-size-boundary=8
	INCLUDE = -I/opt/cegcc/arm-cegcc/gizmondo/include/
	LDFLAGS = -static -L/opt/cegcc/arm-cegcc/gizmondo/lib/
	CXX = arm-cegcc-g++
	STRIP = arm-cegcc-strip
	LIB = /opt/cegcc/arm-cegcc/gizmondo/lib/SDL.dll -lm -lpthread
endif

###

ifdef DEBUG
	CFLAGS += -fpermissive -Wextra -Wall -ggdb3 -c -O2
else
	CFLAGS += -fpermissive -c -O2
endif

SRC =	ai.cpp		\
	all.cpp		\
	apricots.cpp	\
	collide.cpp	\
	drak.cpp	\
	drawall.cpp	\
	fall.cpp	\
	finish.cpp	\
	game.cpp	\
	init.cpp	\
	menu.cpp	\
	sampleio.cpp	\
	SDLfont.cpp	\
	setup.cpp	\
	shape.cpp
	
OBJ = $(SRC:.cpp=.o)
EXE = apricots.elf

build : $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(LIB) -o $@
ifndef DEBUG
	$(STRIP) $(EXE)
endif

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	rm -rf *.o $(EXE)
