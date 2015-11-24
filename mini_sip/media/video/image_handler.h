#ifndef IMAGE_HANDLER_H
#define IMAGE_HANDLER_H

#include "my_types.h"
#include "sobject.h"
#include <string.h>

extern "C"
{
    #include "libavutil/mem.h"
}

#ifdef WORDS_BIGENDIAN
#   define FOURCC( a, b, c, d ) \
        ( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) \
           | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )

#else
#   define FOURCC( a, b, c, d ) \
        ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
           | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )

#endif

#define M_CHROMA_I420           FOURCC( 'I', '4', '2', '0' )
#define M_CHROMA_YUYV           FOURCC( 'Y', 'U', 'Y', 'V' )
#define M_CHROMA_UYVY           FOURCC( 'U', 'Y', 'V', 'Y' )
#define M_CHROMA_RV16           FOURCC( 'R', 'V', '1', '6' )
#define M_CHROMA_RV24           FOURCC( 'R', 'V', '2', '4' )
#define M_CHROMA_RV32           FOURCC( 'R', 'V', '3', '2' )

typedef struct MData
{
    uint8_t *data[4];
    int linesize[4];
} MData;

static void copySImagePlaneWithoutRightPadding(uint8_t *src, const int &srcLinesize, uint8_t *dst, const int &dstLinesize, const int &height)
{
    my_assert(dstLinesize <= srcLinesize);
    for(int i=0; i<height; ++i)
    {
        memcpy(dst, src, dstLinesize);
        src += srcLinesize;
        dst += dstLinesize;
    }
}

class SImage : public SObject
{
public:
    SImage() : uTime(-1), private_data(NULL)
    {
        data[0] = data[1] = data[2] = data[3] = NULL;
        linesize[0] = linesize[1] = linesize[2] = linesize[3] = 0;
        free_buffers=false;
    }

    virtual ~SImage()
    {
        if (free_buffers)
        {
            if (data[0])
                delete[] data[0];
        }
    }

    static SRef<SImage*> factory(const int &width, const int &height, const int &pixelFormat, uint8_t *(*data)[4]=NULL, int (*linesize)[4]=NULL)
    {
        SRef<SImage*> image = new SImage();
        image->free_buffers = true;
        image->width = width;
        image->height = height;
        image->chroma = pixelFormat;
        switch(pixelFormat)
        {
        case M_CHROMA_YUYV:
        case M_CHROMA_UYVY:
            image->data[0] = (uint8_t*) av_malloc(width * height * 2);
            image->linesize[0] = width * 2;
            if(data && linesize)
                copySImagePlaneWithoutRightPadding((*data)[0], (*linesize)[0], image->data[0], image->linesize[0], image->height);
            break;
        case M_CHROMA_RV32:
            image->data[0] = (uint8_t*) av_malloc(width * height * 4);
            image->linesize[0] = width * 4;
            if(data && linesize)
                copySImagePlaneWithoutRightPadding((*data)[0], (*linesize)[0], image->data[0], image->linesize[0], image->height);
            break;
        case M_CHROMA_RV24:
            image->data[0] = (uint8_t*) av_malloc(width * height * 3);
            image->linesize[0] = width * 3;
            if(data && linesize)
                copySImagePlaneWithoutRightPadding((*data)[0], (*linesize)[0], image->data[0], image->linesize[0], image->height);
            break;
        default: // M_CHROMA_I420
            image->data[0] = (uint8_t*) av_malloc(width * height * 3 / 2);
            image->linesize[0] = width;
            image->data[1] = image->data[0] + image->linesize[0] * height;
            image->linesize[1] = image->linesize[0] / 2;
            image->data[2] = image->data[1] + image->linesize[1] * height / 2;
            image->linesize[2] = image->linesize[1];
            if(data && linesize)
            {
                copySImagePlaneWithoutRightPadding((*data)[0], (*linesize)[0], image->data[0], image->linesize[0], image->height);
                copySImagePlaneWithoutRightPadding((*data)[1], (*linesize)[1], image->data[1], image->linesize[1], image->height / 2);
                copySImagePlaneWithoutRightPadding((*data)[2], (*linesize)[2], image->data[2], image->linesize[2], image->height / 2);
            }
            break;
        }
        return image;
    }

    uint8_t *data[4];
    int linesize[4];
    uint32_t ssrc;
    uint32_t chroma;
    int64_t uTime;
    void * private_data;
    uint32_t width;
    uint32_t height;
    bool free_buffers;
};

class Image_Handler : public virtual SObject
{
public:
    virtual void handle( const SRef<SImage *>& ) = 0;
};

#endif // IMAGE_HANDLER_H
