# Define compilation type
#OSTYPE?=msys
#OSTYPE=oda320
OSTYPE=odgcw0

PRGNAME     = spout-od

# define regarding OS, which compiler to use
ifeq "$(OSTYPE)" "msys"	
EXESUFFIX = .exe
TOOLCHAIN = /c/MinGW32
CC          = gcc
LD          = gcc
else
ifeq "$(OSTYPE)" "oda320"	
TOOLCHAIN = /opt/rs97-toolchain/bin/
EXESUFFIX = .dge
else
TOOLCHAIN = /opt/rs97-toolchain/bin/
EXESUFFIX = .dge
endif

EXESUFFIX ?= .elf
CC = /opt/rs97-toolchain/bin/mipsel-linux-gcc
LD = /opt/rs97-toolchain/bin/mipsel-linux-gcc
endif

# add SDL dependencies
SDL_LIB     = $(TOOLCHAIN)/lib
SDL_INCLUDE = $(TOOLCHAIN)/include

# change compilation / linking flag options
ifeq "$(OSTYPE)" "msys"	
  CFLAGS      = -I$(SDL_INCLUDE) -O2 
  LDFLAGS     = -L$(SDL_LIB) -L. -lmingw32 -lSDLmain -lSDL -mwindows
else
  F_OPTS = -falign-functions -falign-loops -falign-labels -falign-jumps \
	  -ffast-math -fsingle-precision-constant -funsafe-math-optimizations \
	  -fomit-frame-pointer -fno-builtin -fno-common \
	  -fstrict-aliasing  -fexpensive-optimizations \
	  -finline -finline-functions -fpeel-loops

ifeq "$(OSTYPE)" "oda320"	
  CC_OPTS	= -O2 -mips32 -mhard-float -G0 $(F_OPTS)
else
  CC_OPTS	= -O2 -mips32 -mhard-float -G0 $(F_OPTS)
endif

CFLAGS      = -DOPENDINGUX $(CC_OPTS)
LDFLAGS     = $(CC_OPTS) -lSDL 
endif

# Files to be compiled
OBJS        = spout.o piece.o

# Rules to make executable
$(PRGNAME)$(EXESUFFIX): $(OBJS)
ifeq "$(OSTYPE)" "msys"	
	$(LD) $(CFLAGS) -o $(PRGNAME)$(EXESUFFIX) $^ $(LDFLAGS)
else
	$(LD) -s $(LDFLAGS) -o $(PRGNAME)$(EXESUFFIX) $^
endif

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

spout.o: piece.h font.h sintable.h
piece.c: piece.h

opk:
	rm -rf opk_dir
	cp -r opk_data opk_dir
	cp $(PRGNAME)$(EXESUFFIX) opk_dir
	mksquashfs opk_dir spout.opk -all-root -noappend -no-exports -no-xattrs

opkclean:
	rm -rf opk_dir spout.opk

clean:
	rm -f $(PRGNAME)$(EXESUFFIX) *.o
