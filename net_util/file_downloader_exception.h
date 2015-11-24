#ifndef FILE_DOWNLOADER_EXCEPTION_H
#define FILE_DOWNLOADER_EXCEPTION_H

#include "exception.h"

class File_Downloader_Exception : public Exception
{
public:
    File_Downloader_Exception();
    File_Downloader_Exception(std::string message);
    virtual ~File_Downloader_Exception() throw() {}
    virtual const char *what() const throw();
protected:
    std::string msg;
};

class File_Downloader_Not_Found_Exception : public File_Downloader_Exception
{
public:
    File_Downloader_Not_Found_Exception(std::string message);
};

#endif // FILE_DOWNLOADER_EXCEPTION_H
