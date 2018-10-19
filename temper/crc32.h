#ifndef CRC32_H
#define CRC32_H
int Crc32_ComputeFile( FILE *file, unsigned long *outCrc32 );
unsigned long Crc32_ComputeBuf( unsigned long inCrc32, const void *buf, size_t bufLen );
#endif
