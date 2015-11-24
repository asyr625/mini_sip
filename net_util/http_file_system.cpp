#include "http_file_system.h"
#include "http_downloader.h"

#include<string.h>
#include<stdlib.h>

using namespace std;

class File_String : public File
{
public:
    File_String(char *data, int len, SRef<Stream_Socket*> ssock);
    virtual int32_t read(void *buf, int32_t count);
    virtual void write(void *buf, int32_t count);
    virtual bool eof();
    virtual void seek(int64_t pos);
    virtual int64_t offset();
    virtual int64_t size();
    virtual void flush();

private:
    char *data;
    int64_t len;
    int64_t curpos;
    bool dirty;
    int64_t allocSize;
    SRef<Stream_Socket*> ssock;

};

File_String::File_String( char *d, int l, SRef<Stream_Socket*> ssock_ )
    : data(d), len(l),
      curpos(0), dirty(false),
      allocSize(len), ssock(ssock_)
{ }


int32_t File_String::read(void *buf, int32_t count)
{
    if ( count+curpos > len )
    {
        count = len-curpos;
    }
    memcpy(buf, &data[curpos], count);
    curpos += count;
    return count;
}

void File_String::write(void *buf, int32_t count)
{
    if( curpos + count > allocSize)
    {
        allocSize =  curpos + count + (curpos + count) / 4;
        data = (char*)realloc(data, allocSize ); // alloc 25% more than needed
    }

    memcpy( &data[curpos], buf, count );

    curpos+=count;
    if (curpos>len)
        len = curpos;
}

bool File_String::eof()
{
    return curpos>=len;
}


void File_String::seek(int64_t pos )
{
    curpos = pos;
    if (curpos>len)
        curpos=len;
}

int64_t File_String::offset()
{
    return curpos;
}

int64_t File_String::size()
{
    return len;
}

void File_String::flush()
{
    //TODO: FIXME:  upload to Webdav/HTTP server not implemented
}


Http_File_System::Http_File_System( SRef<Stream_Socket*> conn_, std::string prefix_ )
    :prefix(prefix_), conn(conn_)
{ }

void Http_File_System::mkdir( const std::string & name )
{
}

SRef<File*> Http_File_System::open( const std::string& path, bool createIfNotExist )
{
    char *data=NULL;
    int len=0;
    Http_Downloader dl( path , conn ); //FIXME: Check if "path" makes sense to send here
    data = dl.get_chars(&len);
    return new File_String( data, len, conn);
}
