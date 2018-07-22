/*  This file is based on FileList.cpp from Oldplay's soure code written
 by Jonas Minnberg.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#include "file_list.h"

#define MAX_FILE_NAME 128

using namespace std;

/* Util functions */
const char *BaseName(const char *fname) {
	const char *ptr = &fname[strlen(fname) - 1];
	while (ptr > fname && *ptr != ':' && *ptr != '\\' && *ptr != '/')
		ptr--;
	if (ptr != fname)
		ptr++;
	return ptr;
}

int is_ext(const char *s, const char *ext) {
	const char *sext = strrchr(s, '.');
	return sext && strcasecmp(ext, sext) == 0;
}

int compare(const void* a, const void* b) {
	return strcasecmp((const char *) a, (const char *) b);
}

/* Class functions */
int FileList::filter_cb(const char *name) {
	for (int i = 0; file_types[i] != NULL; i++)
		if (is_ext(name, file_types[i]))
			return 1;
	return 0;
}

FileList::FileList(char *dirname, const char *types[]) {
	file_types = types;

	if (dirname) {
		strcpy(curdir, dirname);
		Enter(dirname);
	} else
		*curdir = 0;
}

void FileList::AddDirectory(char *dirname, bool recursive) {
	DIR *dir = opendir(dirname);
	if (dir) {
		char tmp[MAX_FILE_NAME];
		int count = 0;
		int folders = 0, files = 0;
		for (struct dirent *de; de = readdir(dir);) {
			struct stat ss;
			ss.st_mode = 0;
			sprintf(tmp, "%s%c%s", dirname, SEPARATOR, de->d_name);
			if (stat(tmp, &ss) == 0 && de->d_name[0] != '.') {
				if (ss.st_mode & S_IFDIR)
					folders++;
				else
					files++;
			}
		}
		rewinddir(dir);
		char sorted[folders + files][MAX_FILE_NAME];
		int k = 0, m = 0, n = 0;
		for (struct dirent *de; de = readdir(dir);) {
			struct stat ss;
			ss.st_mode = 0;
			sprintf(tmp, "%s%c%s", dirname, SEPARATOR, de->d_name);
			if (stat(tmp, &ss) == 0 && de->d_name[0] != '.') {
				if (ss.st_mode & S_IFDIR)
					strcpy(sorted[k++], de->d_name);
				else
					strcpy(sorted[folders + n++], de->d_name);
			}
		}

		qsort(sorted, folders, MAX_FILE_NAME, &compare);
		qsort(&sorted[folders], files, MAX_FILE_NAME, &compare);

		for (k = 0, m = folders + files; k < m; k++) {
			char *tmp2 = sorted[k];
			struct stat ss;
			ss.st_mode = 0;
			sprintf(tmp, "%s%c%s", dirname, SEPARATOR, tmp2);
			if (stat(tmp, &ss) == 0) {
				if ((ss.st_mode & S_IFDIR) || filter_cb(tmp2)) {
					if (recursive && (ss.st_mode & S_IFDIR))
						AddDirectory(tmp, recursive);
					else {
						FileData fd;
						fd.path = string(tmp);
						fd.name = string(BaseName(tmp));
						if (ss.st_mode & S_IFDIR)
							fd.size = -1;
						else
							fd.size = ss.st_size;
						filerefs.push_back(fd);
					}
				}
			}
		}
		closedir(dir);
	}
}

void FileList::Enter(char *dirname) {
	filerefs.clear();
	AddDirectory(dirname);
}

int FileList::Enter(int index) {
	char old[MAX_FILE_NAME] = "";
	if (index == -1) {
		char *end = strrchr(curdir, SEPARATOR);
		if (end) {
			strcpy(old, curdir);
			*end = 0;
			if (!strchr(curdir, SEPARATOR)) {
				*end = SEPARATOR;
				end[1] = 0;
			}
		}
	} else
		strcpy(curdir, filerefs[index].path.c_str());

	Enter(curdir);

	if (strlen(old)) {
		for (unsigned int i = 0; i < filerefs.size(); i++) {
			if (strcmp(filerefs[i].path.c_str(), old) == 0) {
				return i;
			}
		}
	}

	return 0;

}

