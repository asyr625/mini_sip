#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "sobject.h"
#include "exception.h"

#include "my_types.h"

class File_Exception : public Exception
{
public:
    File_Exception(const std::string why);
};

class File_System_Exception : public Exception
{
public:
    File_System_Exception(const std::string why);
};

class File : public SObject
{
public:
    virtual int32_t read(void *buf, int32_t count) = 0;
    virtual void write(void *buf, int32_t count) = 0;

    virtual bool eof() = 0;

    virtual void seek(int64_t pos ) = 0;

    virtual int64_t offset() = 0;
    virtual int64_t size() = 0;
    virtual void flush() = 0;
};

class File_System : public SObject
{
public:
    virtual void mkdir( const std::string & name ) = 0;
    virtual SRef<File*> open( const std::string& path, bool createIfNotExist=false ) = 0;

    virtual void set_default_path(std::string);
    virtual std::string get_default_path();
protected:
    std::string def_prefix;
};

class Local_File_System : public File_System
{
public:
    virtual void mkdir( const std::string & name );
    virtual SRef<File*> open( const std::string& path, bool createIfNotExist=false );
};

#endif // FILE_SYSTEM_H
