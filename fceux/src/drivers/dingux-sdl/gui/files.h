/*  This file is based on Files.h from Oldplay's soure code written
    by Jonas Minnberg.
    */

#ifndef FILES_H
#define FILES_H

#include <vector>
#include <string>

#define SEPARATOR '/'

struct FileData
{
    std::string name;
    std::string path;
    int size;
};

class IFileList
{
    public:
        IFileList() {}
        virtual ~IFileList() {}

        virtual int         Size()              = 0;
        virtual const char *GetName(int index)  = 0;
        virtual const char *GetPath(int index)  = 0;

        virtual int   Enter(int index) = 0;
    protected:
        std::vector<FileData> filerefs;
};

#endif
