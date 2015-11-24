#ifndef FILE_DISPLAY_H
#define FILE_DISPLAY_H

#include "video_media.h"
#include "image_handler.h"
#include "video_display.h"

class File_Display : Video_Display
{
public:
    File_Display(std::string callId, uint32_t width, uint32_t height );

    void open_display();
    void init( uint32_t width, uint32_t height );

    SRef<SImage *> allocate_image();
    bool handles_chroma( uint32_t chroma );
    void display_image( SImage * mimage );
    void resize(int w, int h);

    void set_is_local_video(bool isLocal) { local = isLocal; }
    bool get_is_local_video() { return local; }
    /* From ImageHandler */

protected:
    bool local;
    uint32_t height;
    uint32_t width;
    uint32_t base_window_width;
    uint32_t base_window_height;

    int screen;
    int screen_depth;
private:
    int fd;
    std::string fname;
    uint32_t bytes_per_pixel;
};


class File_Display_Plugin: public Video_Display_Plugin
{
public:
    File_Display_Plugin( SRef<Library *> lib ): Video_Display_Plugin( lib ){}

    virtual std::string get_name() const { return "filedisplay"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "File display"; }

    virtual SRef<Video_Display *> create(SRef<Video_Media*> media, uint32_t width, uint32_t height, bool fullscreen, std::string callId ) const
    {
        return new File_Display( media, callId, width, height );
    }

    virtual std::string get_mem_object_type() const { return "FilePlugin"; }
};

#endif // FILE_DISPLAY_H
