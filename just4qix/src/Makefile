CC=/opt/rs97-toolchain/bin/mipsel-linux-gcc
CFLAGS=-O3 -ffast-math -mips32
LIBS=-lSDL -lSDL_mixer -s

just4qix: main.o render.o update.o input.o sound.o particle.o
	$(CC) main.o render.o update.o input.o sound.o particle.o -o just4qix -lSDL -lSDL_mixer -s

clean:
	rm -rf *.o
	rm -rf just4qix
