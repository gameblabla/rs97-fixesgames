/*  This file is based on FileList.h from Oldplay's soure code written
    by Jonas Minnberg.
    */

#ifndef FILELIST_H
#define FILELIST_H

#include "files.h"

class FileList : public IFileList
{
    public:
        FileList(char *source, const char *types[]);
        virtual ~FileList() {}

        virtual const char *GetName(int index)  { return filerefs[index].name.c_str(); }
        virtual const char *GetPath(int index)  { return filerefs[index].path.c_str(); }
        virtual int         GetSize(int index)  { return filerefs[index].size; }
        int                 Size()              { return filerefs.size(); }
        virtual int Enter(int index);
        char *GetCurDir() { return curdir; }

    protected:
        void AddDirectory(char *dirname, bool recursive = false);
        void Enter(char *dirname);
        int filter_cb(const char *);
      //  void *cb_data;
      //  bool dirty;
      //  int changed;
        char curdir[256];
        const char **file_types;
};

#endif
