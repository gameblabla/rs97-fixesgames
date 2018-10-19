#ifndef PSP_RAM_H
#define PSP_RAM_H

//#include <psptypes.h>
typedef unsigned long u32;
typedef unsigned char u8;

#define RAM_BLOCK      (1024 * 1024)



class psp_ram
{
public:
    psp_ram();
    u32 ramAvailableLineareMax();
    u32 ramAvailable();
};

#endif // PSP_RAM_H
