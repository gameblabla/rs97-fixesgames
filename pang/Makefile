CFLAGS += -DPC_VERSION -DRES320X240 `/home/steward/Downloads/buildroot-2017.02.9/output/host/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/bin/sdl-config --cflags`
all: pang
pang : main.o structures.o ball.o collisions.o plateforme.o player.o ladder.o bonus.o shoot.o objets.o levels.o PCGfxEngine.o PCSoundEngine.o
	/opt/rs97-toolchain/bin/mipsel-linux-gcc  -lSDL -lSDL_gfx -lSDL_mixer -lSDL_image -o pang main.o structures.o ball.o collisions.o plateforme.o player.o ladder.o bonus.o shoot.o objets.o levels.o PCGfxEngine.o PCSoundEngine.o
.c.o: 
	/opt/rs97-toolchain/bin/mipsel-linux-gcc $(CFLAGS) -c $*.c -o $*.o 

clean:
	rm -rf *.o
	rm -rf pang 
	rm -rf *~

.PHONY: clean pang
