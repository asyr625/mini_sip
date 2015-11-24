#include "threaded_frame_converter.h"

#include "video_display.h"
#include "video_media.h"
#include "my_time.h"

extern "C" {
#include<libswscale/swscale.h>
}
#include<iostream>

Threaded_Frame_Converter::Threaded_Frame_Converter(SRef<Video_Media_Source*> videoMediaSource, const std::string &_name, const bool &_dropFramesWhenBehind, const bool &_sinkDestroysImages)
    : video_media_source(videoMediaSource), name(_name), drop_frames_when_behind(_dropFramesWhenBehind), sink_destroys_images(_sinkDestroysImages),
      thread(NULL), sws_context(NULL), quit(false), current_dest_width(0), current_dest_height(0), current_dest_simage_pixel_format(0), converted_image(NULL)
{

}

Threaded_Frame_Converter::~Threaded_Frame_Converter()
{
    stop();
    quit = true;
    if(sws_context)
        sws_freeContext(swsContext);
    //MImage::destroy(convertedImage);
    converted_image = NULL;
    for(std::deque<ThreadedFrameConverterQueueElement>::iterator it=image_queue.begin(), end=image_queue.end(); it!=end; ++it)
    {
        //SImage::destroy(it->image);
        it->image = NULL;
    }
    std::cout << "Threaded_Frame_Converter \"" << name << "\" deleted" << std::endl;
}

void Threaded_Frame_Converter::handle(const SRef<SImage *>& image, const int &destWidth, const int &destHeight, const int &destSImagePixelFormat, const bool &makeLocalCopyFirst)
{
    //merr << "ThreadedFrameConverter \"" << name << "\" handle() started at " << mtime() << "ms" << std::endl;
    SRef<SImage *> localImage;
    if(makeLocalCopyFirst)
    {
        localImage = SImage::factory(image->width, image->height, image->chroma, &image->data, &image->linesize);
        localImage->uTime = image->uTime;
        localImage->ssrc = image->ssrc;
    }
    else
    {
        localImage = image;
    }
    queue_mutex.lock();
    if(drop_frames_when_behind && !image_queue.empty()) // permissive approach
    {
        std::deque<ThreadedFrameConverterQueueElement>::iterator first = image_queue.begin();
        if(first->local_copy)
        {
            //SImage::destroy(first->image);
            first->image = NULL;
        }
        image_queue.erase(first);
    }
    else
        queue_semaphore.inc();
    image_queue.push_back(ThreadedFrameConverterQueueElement(localImage, destWidth, destHeight, destSImagePixelFormat, makeLocalCopyFirst));
    queue_mutex.unlock();
    if(thread == NULL && quit == false)
        thread = new Thread(this, Thread::Normal_Priority);
    //merr << "Threaded_Frame_Converter \"" << name << "\" handle() finished at " << mtime() << "ms" << std::endl;
}

void Threaded_Frame_Converter::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Threaded_Frame_Converter::run");
#endif
    while(1)
    {
        //my_err << "Threaded_Frame_Converter \"" << name << "\" single run() pass started at " << mtime() << "ms" << std::endl;
        queue_semaphore.dec();
        if(quit)
            return;
        queue_mutex.lock();
        my_assert(!image_queue.empty());
        ThreadedFrameConverterQueueElement firstElement = image_queue.front();
        image_queue.pop_front();
        queue_mutex.unlock();
        if(firstElement.dest_width != current_dest_width || firstElement.dest_height != current_dest_height
                || firstElement.dest_simage_pixel_format != current_dest_simage_pixel_format || sws_context == NULL || converted_image.is_null() )
        {
            current_dest_width = firstElement.dest_width;
            current_dest_height = firstElement.dest_height;
            current_dest_simage_pixel_format = firstElement.dest_simage_pixel_format;
            if(sws_context)
                sws_freeContext(sws_context);
            sws_context = sws_getContext(firstElement.image->width, firstElement.image->height, mImagePixelFormatToLibavcodecPixelFormat(firstElement.image->chroma),
                                        current_dest_width, current_dest_height, mImagePixelFormatToLibavcodecPixelFormat(current_dest_simage_pixel_format),
                                        SWS_FAST_BILINEAR, NULL, NULL, NULL);
            //SImage::destroy(convertedImage);
            converted_image = NULL;
            converted_image = SImage::factory(current_dest_width, current_dest_height, current_dest_simage_pixel_format);
        }
        converted_image->uTime = firstElement.image->uTime;
        converted_image->ssrc = firstElement.image->ssrc;
        //		merr << "Threaded_Frame_Converter \"" << name << "\" run() scaling started at " << mtime() << "ms" << std::endl;
        sws_scale(sws_context, firstElement.image->data, firstElement.image->linesize, 0, firstElement.image->height, converted_image->data, converted_image->linesize);


        //It looks like there is a bug in swscale 0.7.1 which makes every second row have pixels swapped horizontally in pairs...
#if LIBSWSCALE_VERSION_MAJOR==0 && LIBSWSCALE_VERSION_MINOR==7 && LIBSWSCALE_VERSION_MICRO==1
        for (int y=1; y<converted_image->height; y+=2)
            for (int x=0; x<converted_image->width; x+=2)
            {
                uint8_t r1 = converted_image->data[0][x*4 + y*converted_image->linesize[0]+0];
                uint8_t g1 = converted_image->data[0][x*4 + y*converted_image->linesize[0]+1];
                uint8_t b1 = converted_image->data[0][x*4 + y*converted_image->linesize[0]+2];

                uint8_t r2 = converted_image->data[0][(x+1)*4 + y*converted_image->linesize[0]+0];
                uint8_t g2 = converted_image->data[0][(x+1)*4 + y*converted_image->linesize[0]+1];
                uint8_t b2 = converted_image->data[0][(x+1)*4 + y*converted_image->linesize[0]+2];
                converted_image->data[0][(x+1)*4 + y*converted_image->linesize[0]+0]=r1;
                converted_image->data[0][(x+1)*4 + y*converted_image->linesize[0]+1]=g1;
                converted_image->data[0][(x+1)*4 + y*converted_image->linesize[0]+2]=b1;

                converted_image->data[0][x*4 + y*converted_image->linesize[0]+0]=r2;
                converted_image->data[0][x*4 + y*converted_image->linesize[0]+1]=g2;
                converted_image->data[0][x*4 + y*converted_image->linesize[0]+2]=b2;

            }
#endif
        //my_err << "Threaded_Frame_Converter \"" << name << "\" run() scaling finished at " << mtime() << "ms" << std::endl;
#if 0
        if (sink)
        {
            sink->handle(converted_image);
        }else{
            merr <<"WARNING: Threaded_Frame_Converter::run: sink is null - image dropped"<<std::endl;
        }
#endif
        video_media_source->frame_converter_output(converted_image);

        //my_err << "Threaded_Frame_Converter \"" << name << "\" run() sink->handle finished at " << mtime() << "ms" << std::endl;
        if(sink_destroys_images)
            converted_image = SImage::factory(current_dest_width, current_dest_height, current_dest_simage_pixel_format);
        if(firstElement.local_copy)
        {
            //SImage::destroy(firstElement.image);
            firstElement.image=NULL;
        }
        //my_err << "Threaded_Frame_Converter \"" << name << "\" single run() pass finished at " << mtime() << "ms" << std::endl;
    }
}

void Threaded_Frame_Converter::stop()
{
    if(thread)
    {
        quit = true;
        queue_semaphore.inc();
        thread->join();
        std::cout << "Threaded_Frame_Converter \"" << name << "\" stopped its thread" << std::endl; // at " << mtime() << "ms" << std::endl;
        delete thread;
        thread = NULL;
        quit = false;
    }
}

void Threaded_Frame_Converter::start()
{
}

PixelFormat Threaded_Frame_Converter::mImagePixelFormatToLibavcodecPixelFormat(const int &mImagePixelFormat)
{
    switch(mImagePixelFormat)
    {
    case M_CHROMA_YUYV:
        return PIX_FMT_YUYV422;
    case M_CHROMA_UYVY:
        return PIX_FMT_UYVY422;
    case M_CHROMA_RV32:
        return PIX_FMT_RGB32;
    case M_CHROMA_RV24:
        return PIX_FMT_RGB24;
    default:
        return PIX_FMT_YUV420P;
    }
}
