#include "file_display.h"

#include "video_exception.h"

#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

static std::list<std::string> pluginList;
static bool initialized;

extern "C"
std::list<std::string> *filedisplay_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * filedisplay_LTX_getPlugin( SRef<Library*> lib )
{
    return new File_Display_Plugin( lib );
}
File_Display::File_Display(std::string callId, uint32_t width, uint32_t height)
    : Video_Display(callId)
{
    screen_depth = 24;
    bytes_per_pixel = 3;
    this->width = width;
    this->height = height;
    fd = -1;
    fname = "/dev/null";
}

void File_Display::open_display()
{
    fd = open(fname.c_str(),O_WRONLY);
}

void File_Display::init( uint32_t width, uint32_t height )
{
    this->width = width;
    this->height = height;
}

SRef<SImage *> File_Display::allocate_image()
{
    cerr <<"EEEE: allocating image of size "<<width<<"x"<<height<<"x"<<bytes_per_pixel<<endl;
    char * imageData = ( char * )calloc( width * height * bytes_per_pixel+16 +1000000 ,1);

    SRef<SImage *> mimage;

    mimage = new SImage;
    mimage->width = width;
    mimage->height = height;
    mimage->uTime = 0;

    fprintf( stderr, "bytesPerPixel: %i\n",  bytes_per_pixel );
    //for( unsigned int i = 0; i < 3; i++ ){
    mimage->data[0] = (uint8_t *)(imageData);
    mimage->data[1] = NULL;
    mimage->data[2] = NULL;
    mimage->linesize[0] = width*bytes_per_pixel;
    mimage->linesize[1] = 0;
    mimage->linesize[2] = 0;
    //}

    switch( screen_depth )
    {
    case 16:
        mimage->chroma = M_CHROMA_RV16;
        break;
    case 24:
    case 32:
    default:
        mimage->chroma = M_CHROMA_RV32;
        break;
    }
    return mimage;
}

bool File_Display::handles_chroma( uint32_t chroma )
{
    switch( screen_depth )
    {
    case 16:
        return chroma == M_CHROMA_RV16;
    case 24:
    case 32:
        return chroma == M_CHROMA_RV32;
    default:
        return false;
    }
}


void File_Display::display_image( SImage * mimage )
{
    //write(fd,mimage->...);
}

void File_Display::resize(int w, int h)
{
    stop();
    init(w,h);
    start();
}
