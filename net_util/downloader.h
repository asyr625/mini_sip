#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "sobject.h"
#include "stream_socket.h"

class Downloader : public SObject
{
public:
    static SRef<Downloader*> create(std::string const uri, SRef<Stream_Socket*> conn=NULL);

    virtual std::string get_mem_object_type() const {return "Downloader";}

    virtual char* get_chars(int *length) = 0;
};

#endif // DOWNLOADER_H
