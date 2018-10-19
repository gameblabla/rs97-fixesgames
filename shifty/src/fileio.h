#ifndef _FILEIO_H_
#define _FILEIO_H_

#define FILE_PATH_MAX	255

extern char configFile[FILE_PATH_MAX];

int readConfig();
int storeConfig();

#endif /* _FILEIO_H_ */
