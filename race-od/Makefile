# Define compilation type
#OSTYPE=msys
OSTYPE=opendingux

# define regarding OS, which compiler to use
ifeq "$(OSTYPE)" "msys"	
  EXESUFFIX = .exe
  PRGNAME     = race
  TOOLCHAIN = /c/MinGW32
  CC          = gcc
  LD          = gcc
else
  EXESUFFIX = .dge
  PRGNAME     = race-od
  CC = /opt/rs97-toolchain/bin/mipsel-linux-gcc
  LD = /opt/rs97-toolchain/bin/mipsel-linux-gcc
endif

# add SDL dependencies
SYSROOT     := $(shell $(CC) --print-sysroot)
SDL_CFLAGS  := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS    := -lSDL

# change compilation / linking flag options
ifeq "$(OSTYPE)" "msys"	
  CFLAGS      = -I$(SDL_CFLAGS) -O2 -DZ80 -DTARGET_OD -D_MAX_PATH=2048 -DHOST_FPS=60
  CXXFLAGS=$(CFLAGS) 
  LDFLAGS     = -L$(SDL_LIBS) -lstdc++ -lmingw32 -lSDLmain -lSDL -mwindows
else
  F_OPTS = -falign-functions -falign-loops -falign-labels -falign-jumps \
    -ffast-math -fsingle-precision-constant -funsafe-math-optimizations \
    -fomit-frame-pointer -fno-builtin -fno-common \
    -fstrict-aliasing  -fexpensive-optimizations \
    -finline -finline-functions -fpeel-loops
  CC_OPTS	= -O3 -mips32 -G0 -D_OPENDINGUX_ $(F_OPTS)
  CFLAGS      = -I$(SDL_INCLUDE) -DOPENDINGUX -DZ80 -DTARGET_OD -D_MAX_PATH=2048 -DHOST_FPS=60 $(CC_OPTS)
  CXXFLAGS=$(CFLAGS) 
  LDFLAGS     = -L$(SDL_LIB) $(CC_OPTS) -lstdc++ -lSDL  -s
endif

# Files to be compiled
SRCDIR    = ./emu ./opendingux .
VPATH     = $(SRCDIR)
SRC_C   = $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.cpp))
OBJ_C   = $(notdir $(patsubst %.cpp, %.o, $(SRC_C)))
OBJS     = $(OBJ_C)

# Rules to make executable
$(PRGNAME)$(EXESUFFIX): $(OBJS)  
ifeq "$(OSTYPE)" "msys"	
	$(LD) $(CFLAGS) -o $(PRGNAME)$(EXESUFFIX) $^ $(LDFLAGS)
else
	$(LD) $(LDFLAGS) -o $(PRGNAME)$(EXESUFFIX) $^
endif

$(OBJ_C) : %.o : %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(PRGNAME)$(EXESUFFIX) *.o
