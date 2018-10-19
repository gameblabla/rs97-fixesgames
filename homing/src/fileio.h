#ifndef _FILEIO_H_
#define _FILEIO_H_


#define SAVE_FORMAT_VERSION	1
#define HISCORE_HEADER		"FEVER"
#define HISCORE_FORMAT_VERSION	1
#define FILE_MAX_PATH		100

extern char configDir[FILE_MAX_PATH];

int getConfigDir();
void getConfig();
void storeConfig();
void getHiscore();
void storeHiscore();

#endif /* _FILEIO_H_ */
