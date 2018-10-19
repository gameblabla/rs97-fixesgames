PROG = ZeldaROTH_US.dge
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
CXX  = /opt/rs97-toolchain/bin/mipsel-linux-g++
SDL-CONFIG = /opt/rs97-toolchain/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config

CXXFLAGS = -Wall -Ofast -mno-abicalls -mplt `$(SDL-CONFIG) --cflags` -DDINGUX
LDFLAGS = `$(SDL-CONFIG) --libs` -lSDL_mixer -lSDL_image -lSDL_gfx -flto -s

$(PROG): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(PROG)

clean:
	rm -f $(PROG) *.o

.PHONY: clean
