#ifndef THREADED_FRAME_CONVERTER_H
#define THREADED_FRAME_CONVERTER_H

#include "thread.h"
#include "mutex.h"
#include "my_semaphore.h"
#include "image_handler.h"

#include <deque>
#include <string>
#include <libavutil/pixfmt.h>


class Video_Media_Source;

struct ThreadedFrameConverterQueueElement
{
    SRef<SImage *> image;
    int dest_width, dest_height, dest_simage_pixel_format;
    bool local_copy;

    ThreadedFrameConverterQueueElement(SRef<SImage *> _image=NULL, const int &_destWidth=0, const int &_destHeight=0, const int &_destSImagePixelFormat=0, const bool &_localCopy=true)
        : image(_image), dest_width(_destWidth), dest_height(_destHeight),
          dest_simage_pixel_format(_destSImagePixelFormat), local_copy(_localCopy)
    {
    }
};

class SwsContext;
class Image_Handler;

class Threaded_Frame_Converter : public Runnable
{
public:
    Threaded_Frame_Converter(SRef<Video_Media_Source*> videoMediaSource, const std::string &_name, const bool &_dropFramesWhenBehind, const bool &_sinkDestroysImages);
    virtual ~Threaded_Frame_Converter();
    void handle(const SRef<SImage *>& image, const int &destWidth, const int &destHeight, const int &destSImagePixelFormat, const bool &makeLocalCopyFirst);
    virtual void run();
    void stop();
    void start();

protected:
    PixelFormat mImagePixelFormatToLibavcodecPixelFormat(const int &mImagePixelFormat);

    SRef<Video_Media_Source*> video_media_source;
    std::deque<ThreadedFrameConverterQueueElement> image_queue;
    Thread *thread;
    Mutex queue_mutex;

    Semaphore queue_semaphore;
    SwsContext *sws_context;

    bool quit, drop_frames_when_behind, sink_destroys_images;
    int current_dest_width, current_dest_height, current_dest_simage_pixel_format;
    SRef<SImage *> converted_image;
    std::string name;
};

#endif // THREADED_FRAME_CONVERTER_H
