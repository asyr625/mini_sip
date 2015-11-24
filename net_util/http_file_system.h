#ifndef HTTP_FILE_SYSTEM_H
#define HTTP_FILE_SYSTEM_H

#include "stream_socket.h"
#include "file_system.h"

class Http_File_System : public File_System
{
public:
    Http_File_System(SRef<Stream_Socket*> conn_, std::string prefix_ );
    virtual void mkdir( const std::string & name );
    virtual SRef<File*> open( const std::string& path, bool createIfNotExist=false );

private:
    std::string prefix;
    SRef<Stream_Socket*> conn;
};

#endif // HTTP_FILE_SYSTEM_H
