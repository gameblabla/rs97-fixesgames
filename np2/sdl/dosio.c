#include <sys/stat.h>
#include <unistd.h>
#include "compiler.h"
#include "dosio.h"
#include "pg.h"
#include "psplib.h"

static char curpath[MAX_PATH];
static char *curfilep = curpath;


/* ファイル操作 */

FILEH file_open(const char *path)
{
	return fopen(path,"rwb");
}

FILEH file_open_rb(const char *path)
{
	return fopen(path,"rb");
}

FILEH file_create(const char *path)
{
	return fopen(path,"wb");
}

long file_seek(FILEH handle, long pointer, int method)
{
	fseek(handle,pointer,method);
	return ftell(handle);
}

UINT file_read(FILEH handle, void *data, UINT length)
{
	return fread(data,1,length,handle);
}

UINT file_write(FILEH handle, const void *data, UINT length)
{
	return fwrite(data,1,length,handle);
}

short file_close(FILEH handle)
{
	fclose(handle);
    return(0);
}


#if 0
/** Structure to hold the status information about a file */
typedef struct SceIoStat {
    unsigned int st_mode;
    unsigned int st_attr;
    /** Size of the file in bytes. */
    unsigned int st_size;
    /** Creation time. */
    struct dirent_tm st_ctime;
    /** Access time. */
    struct dirent_tm st_atime;
    /** Modification time. */
    struct dirent_tm st_mtime;
    /** Device-specific data. */
    unsigned int    st_private[6];
} SceIoStat;
#endif

UINT file_getsize(FILEH handle)
{
	UINT length;
	
	long current;
	
	current = ftell(handle);
	fseek(handle,0,SEEK_END);
	length = ftell(handle);
	fseek(handle,current,SEEK_SET);
	
	return length;
}

short file_getdatetime(FILEH handle, DOSDATE *dosdate, DOSTIME *dostime)
{
	return(-1);
}

short file_delete(const char *path)
{
	return(-1);
}

// -1 : 存在しない
short file_attr(const char *path)
{
    return 0;
}

short file_dircreate(const char *path)
{
	return(-1);
}


/* カレントファイル操作 */
void file_setcd(const char *exepath)
{
    file_cpyname(curpath, exepath, sizeof(curpath));
    curfilep = file_getname(curpath);
    *curfilep = '\0';
}

char *file_getcd(const char *path)
{
    file_cpyname(curfilep, path, sizeof(curpath) - (curfilep - curpath));
    return(curpath);
}

FILEH file_open_c(const char *path)
{
    file_cpyname(curfilep, path, sizeof(curpath) - (curfilep - curpath));
    return(file_open(curpath));
}

FILEH file_open_rb_c(const char *path)
{
    file_cpyname(curfilep, path, sizeof(curpath) - (curfilep - curpath));
    return(file_open_rb(curpath));
}

FILEH file_create_c(const char *path)
{
    file_cpyname(curfilep, path, sizeof(curpath) - (curfilep - curpath));
    return(file_create(curpath));
}

short file_delete_c(const char *path)
{
    file_cpyname(curfilep, path, sizeof(curpath) - (curfilep - curpath));
    return(file_delete(curpath));
}

short file_attr_c(const char *path)
{
    file_cpyname(curfilep, path, sizeof(curpath) - (curfilep - curpath));
    return(file_attr_c(curpath));
}

void file_get_execdir(char *path)
{
	getcwd(path,MAX_PATH);
	file_setseparator(path,MAX_PATH);
	strcat(path,"/");
}

static void setflist(struct dirent *dir, FLINFO *fli,int attr_dir)
{
    //    static int y = 3;
	fli->caps = FLICAPS_ATTR;
    fli->size = 0;
    fli->attr = FILEATTR_READONLY; //とりあえず0x01
	
	if (attr_dir)
		fli->attr |= FILEATTR_DIRECTORY;
	
//    cnvdatetime(&w32fd->ftLastWriteTime, &fli->date, &fli->time);
    milstr_ncpy(fli->path, dir->d_name, NELEMENTS(fli->path));
}


BOOL file_listnext(FLISTH hdl, FLINFO *fli);

char listing_dir[MAX_PATH];


FLISTH file_list1st(const char *dir, FLINFO *fli)
{
	DIR *dp;
	
	char olddir[MAX_PATH];
	
	getcwd(olddir,MAX_PATH);
	
	chdir(dir);
	getcwd(listing_dir,MAX_PATH);
	
	chdir(olddir);

	
    dp = opendir(dir);
	
	if (!dp) {
        printf("opendir() failed");
        return FLISTH_INVALID;
    }
	
	if (file_listnext(dp,fli) != SUCCESS)
	{
		closedir(dp);
		return FLISTH_INVALID;
	}
	
	return dp;
}

BOOL file_listnext(FLISTH hdl, FLINFO *fli)
{
	struct stat d_stat;
	struct dirent *ent;
	char path[MAX_PATH];
		
    while((ent = readdir(hdl)))
	{
		if (ent->d_name[0] == '.')
            continue;
		
		strcpy(path,listing_dir);
		file_setseparator(path,MAX_PATH);
		strcat(path,ent->d_name);
		
		stat(path,&d_stat);
		if (S_ISDIR(d_stat.st_mode))
			setflist(ent, fli,1);
		else 
			setflist(ent, fli,0);
		
		return SUCCESS;		
    }
	
	return FAILURE;
}

void file_listclose(FLISTH hdl)
{
	closedir(hdl);
}

void file_catname(char *path, const char *name, int maxlen) {

	int		csize;

	while(maxlen > 0) {
		if (*path == '\0') {
			break;
		}
		path++;
		maxlen--;
	}
	file_cpyname(path, name, maxlen);
	while((csize = milstr_charsize(path)) != 0) {
		if ((csize == 1) && (*path == '\\')) {
			*path = '/';
		}
		path += csize;
	}
}

char *file_getname(const char *path) {

const char	*ret;
	int		csize;

	ret = path;
	while((csize = milstr_charsize(path)) != 0) {
		if ((csize == 1) && (*path == '/')) {
			ret = path + 1;
		}
		path += csize;
	}
	return((char *)ret);
}

void file_cutname(char *path) {

	char	*p;

	p = file_getname(path);
	*p = '\0';
}

char *file_getext(const char *path) {

const char	*p;
const char	*q;

	p = file_getname(path);
	q = NULL;
	while(*p != '\0') {
		if (*p == '.') {
			q = p + 1;
		}
		p++;
	}
	if (q == NULL) {
		q = p;
	}
	return((char *)q);
}

void file_cutext(char *path) {

	char	*p;
	char	*q;

	p = file_getname(path);
	q = NULL;
	while(*p != '\0') {
		if (*p == '.') {
			q = p;
		}
		p++;
	}
	if (q != NULL) {
		*q = '\0';
	}
}

void file_cutseparator(char *path) {

	int		pos;

	pos = strlen(path) - 1;
	if ((pos > 0) &&							// 2文字以上でー
		(path[pos] == '/') &&					// ケツが ¥ でー
		((pos != 1) || (path[0] != '.'))) {		// './' ではなかったら
		path[pos] = '\0';
	}
}

void file_setseparator(char *path, int maxlen) {

	int		pos;

	pos = strlen(path);
	if ((pos) && (path[pos-1] != '/') && ((pos + 2) < maxlen)) {
		path[pos++] = '/';
		path[pos] = '\0';
	}
}

