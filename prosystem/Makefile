# Define compilation type
#OSTYPE=msys
#OSTYPE=oda320
OSTYPE=odgcw0

# define regarding OS, which compiler to use
PRGNAME     = prosystem-od

ifeq "$(OSTYPE)" "msys"	
EXESUFFIX = .exe
TOOLCHAIN = /c/MinGW32
CC          = gcc
LD          = gcc
else
ifeq "$(OSTYPE)" "oda320"	
TOOLCHAIN = /opt/opendingux-toolchain/usr
else
TOOLCHAIN = /opt/rs97-toolchain/usr
endif
EXESUFFIX = .dge
CC = /opt/rs97-toolchain/bin/mipsel-linux-gcc
LD = /opt/rs97-toolchain/bin/mipsel-linux-gcc
endif

# add SDL dependencies
SDL_LIB     = $(TOOLCHAIN)/lib
SDL_INCLUDE = $(TOOLCHAIN)/include

# change compilation / linking flag options
ifeq "$(OSTYPE)" "msys"	
F_OPTS =
CFLAGS      = -I$(SDL_INCLUDE) -O2 -D_VIDOD32_ $(F_OPTS)
LDFLAGS     = -L$(SDL_LIB) -L. -lmingw32 -lSDLmain -lSDL -mwindows
else
F_OPTS = -falign-functions -falign-loops -falign-labels -falign-jumps \
	-ffast-math -fsingle-precision-constant -funsafe-math-optimizations \
	-fomit-frame-pointer -fno-builtin -fno-common \
	-fstrict-aliasing  -fexpensive-optimizations \
	-finline -finline-functions -fpeel-loops
ifeq "$(OSTYPE)" "oda320"	
CC_OPTS	= -Ofast -mno-abicalls -mplt -mips32 -msoft-float -G0 -D_OPENDINGUX_ -D_VIDOD16_ $(F_OPTS)
else
#CC_OPTS	= -O2 -mips32 -mhard-float -G0 -D_OPENDINGUX_ -D_VIDOD32_ $(F_OPTS)
CC_OPTS	= -Ofast -mno-abicalls -mplt -mips32 -mhard-float -G0 -D_OPENDINGUX_ $(F_OPTS)
endif
CFLAGS      = -I$(SDL_INCLUDE) -DOPENDINGUX $(CC_OPTS)
LDFLAGS     = -L$(SDL_LIB) $(CC_OPTS) -lSDL 
endif

# Files to be compiled
SRCDIR    = ./emu/zlib  ./emu ./opendingux
VPATH     = $(SRCDIR)
SRC_C   = $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
OBJ_C   = $(notdir $(patsubst %.c, %.o, $(SRC_C)))
OBJS     = $(OBJ_C)

# Rules to make executable
$(PRGNAME)$(EXESUFFIX): $(OBJS)  
ifeq "$(OSTYPE)" "msys"	
	$(LD) $(CFLAGS) -o $(PRGNAME)$(EXESUFFIX) $^ $(LDFLAGS)
else
	$(LD) $(LDFLAGS) -o $(PRGNAME)$(EXESUFFIX) $^
endif

$(OBJ_C) : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PRGNAME)$(EXESUFFIX) *.o
