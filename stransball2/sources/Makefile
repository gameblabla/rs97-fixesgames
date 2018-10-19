PLATFORM=gcw0

ifeq ($(PLATFORM), gcw0)
CXX	:= /opt/rs97-toolchain/bin/mipsel-linux-g++
STRIP	:= /opt/rs97-toolchain/bin/mipsel-linux-strip
endif

CXX	?= g++
STRIP	?= strip
SYSROOT	:= $(shell $(CXX) --print-sysroot)
TARGET	:= stransball2
OBJS	:= \
	auxiliar.o           configuration.o     encoder.o \
	enemies.o            fonts.o       	 game.o \
	main.o               maps.o              replays.o \
	state_changepack.o   state_chooseship.o  state_endsequence.o \
	state_game.o         state_gameover.o    state_instructions.o \
	state_interphase.o   state_keyredefinition.o \
	state_levelfinished.o state_logo.o       state_mainmenu.o \
	state_replay.o       state_replaymanager.o \
	state_typetext.o     tiles.o             transball.o \
	homedir.o

ifdef DEBUG
	CXXFLAGS += -g3
else
	CXXFLAGS += -Ofast -mno-abicalls -mips32 -mplt -I/home/anonymous/Documents/Dev/master.zip.1-copy2/gh_retrogame_emulator-master/libSGE
endif

all: $(TARGET)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@ -I/opt/rs97-toolchain/mipsel-buildroot-linux-musl/sysroot/usr/include/SDL

# dynamically linked binary:
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ -lSDL -lSDL_image -lSDL_mixer ../../libSGE/libSGE.a
ifndef DEBUG
	$(STRIP) $(TARGET)
endif
	mv ./$(TARGET) ..

clean:
	rm -f ../$(TARGET)
	rm -f *.o
