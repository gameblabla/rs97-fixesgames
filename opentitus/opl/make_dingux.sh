#!/bin/bash

/opt/rs97-toolchain/bin/mipsel-linux-gcc -I. -O2 -g -Wall -I../include -I/usr/include/SDL -I/opt/mipsel-linux-uclibc/usr/include -I/opt/mipsel-linux-uclibc/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -L/opt/mipsel-linux-uclibc/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -D_GNU_SOURCE=1 -D_REENTRANT -MT opl.o -MD -MP -MF .deps/opl.Tpo -c -o opl.o opl.c
mv -f .deps/opl.Tpo .deps/opl.Po
/opt/rs97-toolchain/bin/mipsel-linux-gcc -I. -O2 -g -Wall -I../include -I/usr/include/SDL -I/opt/mipsel-linux-uclibc/usr/include -I/opt/mipsel-linux-uclibc/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -L/opt/mipsel-linux-uclibc/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -D_GNU_SOURCE=1 -D_REENTRANT -MT opl_linux.o -MD -MP -MF .deps/opl_linux.Tpo -c -o opl_linux.o opl_linux.c
mv -f .deps/opl_linux.Tpo .deps/opl_linux.Po
/opt/rs97-toolchain/bin/mipsel-linux-gcc -I. -O2 -g -Wall -I../include -I/usr/include/SDL -I/opt/mipsel-linux-uclibc/usr/include -I/opt/mipsel-linux-uclibc/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -L/opt/mipsel-linux-uclibc/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -D_GNU_SOURCE=1 -D_REENTRANT -MT opl_obsd.o -MD -MP -MF .deps/opl_obsd.Tpo -c -o opl_obsd.o opl_obsd.c
mv -f .deps/opl_obsd.Tpo .deps/opl_obsd.Po
/opt/rs97-toolchain/bin/mipsel-linux-gcc -I. -O2 -g -Wall -I../include -I/usr/include/SDL -I/opt/mipsel-linux-uclibc/usr/include -I/opt/mipsel-linux-uclibc/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -L/opt/mipsel-linux-uclibc/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -D_GNU_SOURCE=1 -D_REENTRANT -MT opl_queue.o -MD -MP -MF .deps/opl_queue.Tpo -c -o opl_queue.o opl_queue.c
mv -f .deps/opl_queue.Tpo .deps/opl_queue.Po
/opt/rs97-toolchain/bin/mipsel-linux-gcc -I. -O2 -g -Wall -I../include -I/usr/include/SDL -I/opt/mipsel-linux-uclibc/usr/include -I/opt/mipsel-linux-uclibc/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -L/opt/mipsel-linux-uclibc/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -D_GNU_SOURCE=1 -D_REENTRANT -MT opl_sdl.o -MD -MP -MF .deps/opl_sdl.Tpo -c -o opl_sdl.o opl_sdl.c
mv -f .deps/opl_sdl.Tpo .deps/opl_sdl.Po
/opt/rs97-toolchain/bin/mipsel-linux-gcc -I. -O2 -g -Wall -I../include -I/usr/include/SDL -I/opt/mipsel-linux-uclibc/usr/include -I/opt/mipsel-linux-uclibc/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -L/opt/mipsel-linux-uclibc/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -D_GNU_SOURCE=1 -D_REENTRANT -MT opl_timer.o -MD -MP -MF .deps/opl_timer.Tpo -c -o opl_timer.o opl_timer.c
mv -f .deps/opl_timer.Tpo .deps/opl_timer.Po
/opt/rs97-toolchain/bin/mipsel-linux-gcc -I. -O2 -g -Wall -I../include -I/usr/include/SDL -I/opt/mipsel-linux-uclibc/usr/include -I/opt/mipsel-linux-uclibc/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -L/opt/mipsel-linux-uclibc/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -D_GNU_SOURCE=1 -D_REENTRANT -MT dbopl.o -MD -MP -MF .deps/dbopl.Tpo -c -o dbopl.o dbopl_light.c
mv -f .deps/dbopl.Tpo .deps/dbopl.Po
rm -f libopl.a
/opt/rs97-toolchain/bin/mipsel-linux-ar cru libopl.a opl.o opl_linux.o opl_obsd.o opl_queue.o opl_sdl.o opl_timer.o dbopl.o 
/opt/rs97-toolchain/bin/mipsel-linux-ranlib libopl.a

