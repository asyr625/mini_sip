#include "avdecoder.h"
#include "video_stream_player.h"
#include "irequest_video_keyframe.h"
#include "video_exception.h"
#include "dbg.h"
#include "my_time.h"

#ifdef LOGGING_SUPPORT
#include "logging_anager.h"
#endif

#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <time.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mem.h>
#include <libswscale/swscale.h>
}
using namespace std;

static void sanitize(char *line)
{
    while(*line)
    {
        if(*line < 0x08 || (*line > 0x0D && *line < 0x20))
            *line='?';
        line++;
    }
}

static void av_log_format_line(void *ptr, int level, const char *fmt, va_list vl,
                               char *line, int line_size, int *print_prefix)
{
    AVClass* avc = ptr ? *(AVClass **) ptr : NULL;
    line[0] = 0;
    if (*print_prefix && avc)
    {
        snprintf(line + strlen(line), line_size - strlen(line), "[%s @ %p] ",
                 avc->item_name(ptr), ptr);
    }

    vsnprintf(line + strlen(line), line_size - strlen(line), fmt, vl);

    *print_prefix = strlen(line) && line[strlen(line) - 1] == '\n';
}

void libavcodec_log(void* ptr, int level, const char* fmt, va_list vl)
{
    static int print_prefix = 1;
    static int count;
    static char prev[1024];
    char line[1024];

    if (level >  av_log_get_level())
        return;

    if (0==strcmp(fmt,"Too many slices, increase MAX_SLICES and recompile\n"))
        return;


    av_log_format_line(ptr, level, fmt, vl, line, sizeof(line), &print_prefix);

    if (print_prefix && !strcmp(line, prev)){
        count++;
        return;
    }
    if (count > 0) {
        fprintf(stderr, "    Last message repeated %d times\n", count);
        count = 0;
    }
    strcpy(prev, line);
    sanitize(line);
    merr<<string("h264decode: ")+line<<endl;
}


int av_lockmgr_callback(void **mutex, enum AVLockOp op)
{
    Mutex** m = (Mutex**)mutex;
    switch (op)
    {
    case AV_LOCK_CREATE:
        my_assert(*mutex==NULL);
        *m = new Mutex;
        break;
    case AV_LOCK_OBTAIN:
        (*m)->lock();
        break;
    case AV_LOCK_RELEASE:
        (*m)->unlock();
        break;
    case AV_LOCK_DESTROY:
        delete *m;
        *mutex=NULL;
        break;
    default:
        my_assert(0);
    }
    return 0;
}

AVDecoder::AVDecoder(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr)
    : Decoder_Instance(rtmsr), realtimeStream(rtmsr), codec( NULL ), context( NULL ), keyframe_request_callback(NULL),
      frame_count(0), last_reported_frame_count(0), timestamp_of_last_framerate_get_ms(0)
{
    decoded_frame_i = 0;
    swsctx = NULL;
    /* Initialize AVcodec */
    avcodec_init();
    avcodec_register_all();

    av_lockmgr_register( av_lockmgr_callback );
    av_log_set_callback(libavcodec_log);

    codec = avcodec_find_decoder( CODEC_ID_H264 );
    if( codec == NULL )
    {
        cerr << "Error: libavcodec does not support H264"<<endl;
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(string("libavcodec does not support H.264"), "error.decoder");
#endif
        throw Video_Exception( "libavcodec does not support H264" );
    }

    context = avcodec_alloc_context();

#ifdef HAVE_MMX
    context->dsp_mask = ( FF_MM_MMX | FF_MM_MMXEXT | FF_MM_SSE );
#endif
//	context->thread_count = 8;
    if( avcodec_open( context, codec ) != 0 )
    {
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(string("Could not open libavcodec codec"), "error.decoder");
#endif
        throw Video_Exception( "Could not open libavcodec codec" );
    }

    context->opaque = this;
//	lastImage = NULL;
//	decodedImage->chroma = M_CHROMA_I420;
    for (int i=0; i<3; i++)
    {
        decoded_frames[i] = avcodec_alloc_frame();
    }
#ifdef LOGGING_SUPPORT
    Logger::get_instance()->info(string("Decoder initialized successfully"), "info.decoder");
#endif
}

AVDecoder::~AVDecoder()
{
    if(swsctx)
        sws_freeContext((SwsContext *)swsctx);
    for (int i=0; i<3; i++)
        av_free(decoded_frames[i]);
}

SRef<Processing_Data*> AVDecoder::decode(const SRef<Rtp_Packet*>& rtp)
{
    return add_packet_to_frame(rtp, false);
}

void AVDecoder::close()
{
}

SRef<Processing_Data*> AVDecoder::add_packet_to_frame(const SRef<Rtp_Packet*>& packet, bool flush)
{
    if (!packet)
        return NULL;

    Rtp_Header &hdr = packet->get_header();

    SRef<Processing_Data*> vdata;
    unsigned char *content = packet->get_content();
    uint32_t clen = packet->get_content_length();
    bool marker = packet->get_header().marker;

    if (!content || !clen)
        return NULL;

    if (index > 3000000 || clen > 1000000)
    {
        index = 0;
        return NULL;
    }

    uint8_t nal = content[0];
    uint8_t type = nal & 0x1f;

    if (!((type>=1 && type<=23) || type==28))
    {
        cerr << "VideoMediaSource::addPacketToFrame: WARNING: unexpected packet type: "<< (int)type<<endl;
    }

    if (type>=1 && type<=23)
    {
        frame[index+0] = 0;
        frame[index+1] = 0;
        frame[index+2] = 1;
        index += 3;
        memcpy( &frame[index] , content , clen  );
        index += clen;
        if (marker)
        {
            vdata = decode_frame(frame, index, packet->get_header().get_timestamp());
            index=0;
        }
    }
    else if (type==28)
    {
        content++; // skip FU indicator
        clen--;

        uint8_t fu_indicator = nal;
        uint8_t fu_header = *content;
        uint8_t start_bit = (fu_header&0x80)>>7;
        uint8_t nal_type = (fu_header&0x1f);
        uint8_t reconstructed_nal = fu_indicator&0xe0;
        uint8_t end_bit = (fu_header&0x40)>>6;

        reconstructed_nal |= (nal_type & 0x1f);

        content++; //skip fu_header
        clen--;

        if (start_bit)
        {
            frame[index+0] = 0;
            frame[index+1] = 0;
            frame[index+2] = 1;
            frame[index+3] = reconstructed_nal;
            index += 4;
        }

        memcpy(&frame[index], content, clen);
        index += clen;
        if (/*end_bit*/ packet->get_header().marker)
        {
            vdata = decode_frame(frame, index, packet->get_header().get_timestamp());
            index = 0;
        }
    }
    return vdata;
}

#define REPORT_N 500
SRef<Processing_Data*> AVDecoder::decode_frame( uint8_t * data, uint32_t length, unsigned int timestamp )
{
    /*
    printf("decodeFrame packet contents: ");
    for(int i=0; i<length; ++i) {
      unsigned char a = ((data[i] & 0x0F0) >> 4) + '0', b = (data[i] & 0x0F) + '0';
      if(a > '9')
        a += 8;
      if(b > '9')
        b += 8;
      printf("%c%c ", a, b);
    }
    printf("\n");
    */
    SRef<Processing_Data_Video*> vdata;

    static struct timeval lasttime;
    static int i=0;
    i++;
    if ( i % REPORT_N == 1)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        int diffms = (now.tv_sec-lasttime.tv_sec)*1000+(now.tv_usec-lasttime.tv_usec)/1000;
        float sec = (float)diffms/1000.0f;
        //printf("%d frames in %fs\n", REPORT_N, sec);
        printf("FPS_DECODE: %f\n", (float)REPORT_N/(float)sec );
        lasttime = now;
        char temp[100];
        sprintf(temp, "%.2f", REPORT_N/sec);
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(temp, "info.decoderFramerate");
#endif
    }

    int ret;
    int gotFrame = 0;

    //	std::cerr << "Decode frame started at " << mtime() << "ms" << std::endl;
#if LIBAVCODEC_VERSION_MAJOR >= 53
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = data;
    pkt.size = length;
    ret = avcodec_decode_video2( context, decoded_frames[decoded_frame_i], &gotFrame, &pkt);
#else
    ret = avcodec_decode_video( context, decoded_frames[decoded_frame_i], &gotFrame, data, length );
#endif
    if(ret < 0)
        try_requesting_keyframe();

    if( gotFrame )
    {
        vdata = new Processing_Data_Video;
        vdata->image = new SImage;
        for(int i=0; i<4; ++i)
        {
            vdata->image->data[i] = decoded_frames[decoded_frame_i]->data[i];
            vdata->image->linesize[i] = decoded_frames[decoded_frame_i]->linesize[i];
        }
        vdata->image->uTime = timestamp;	// VideoStreamPlayer::handle(MImage *image) requires the image->uTime to contain a raw rtp timestamp
        vdata->image->width = context->width;
        vdata->image->height= context->height;
        vdata->image->chroma= M_CHROMA_I420;
        vdata->image->free_buffers = false; //we point to local data allocated by av_alloc
        ++frame_count;
        decoded_frame_i = (decoded_frame_i+ 1 ) % 3;
    }

#ifdef DEBUG_OUTPUT
    if (i % 200 == 0)
    {
        static struct timespec last_wallt;
        static struct timespec last_cput;

        struct timespec now_cpu;
        struct timespec now_wall;
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now_cpu);
        clock_gettime(CLOCK_REALTIME, &now_wall);
        cerr <<"=======> AVDecoder:: CPU USAGE: "<< now_cpu.tv_sec<<"."<<now_cpu.tv_nsec<<endl;
        uint64_t delta_cpu = (now_cpu.tv_sec-last_cput.tv_sec)*1000000000LL+(now_cpu.tv_nsec-last_cput.tv_nsec);
        cerr << "Last 200 frames took "<< delta_cpu/1000 <<"us"<<endl;
        uint64_t delta_wall = (now_wall.tv_sec-last_wallt.tv_sec)*1000000000LL+(now_wall.tv_nsec-last_wallt.tv_nsec);

        last_cput  = now_cpu;
        last_wallt = now_wall;
        cerr <<"========> AVDecoder CPU usage: "<< ((float)delta_cpu/(float)delta_wall)*100.0<<"%"<<endl;
        char temp[100];
        sprintf(temp, "%f ", ((float)delta_cpu/(float)delta_wall)*100.0);
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(temp, "info.decoderCpu");
#endif
    }
#endif

    if (vdata)
        return *vdata;
    else
        return NULL;
}

void AVDecoder::set_ssrc( uint32_t ssrc )
{
    this->ssrc = ssrc;
}

std::string AVDecoder::get_description()
{
    std::string description = "H.264/AVC";
    if(context) {
        char tmp[30];
        sprintf(tmp, "%ux%u", context->width, context->height);
        description += ", ";
        description += tmp;
    }
    return description;
}

float AVDecoder::get_current_framerate()
{
    float result = 0;
    uint64_t now_ms = mtime();
    if(timestamp_of_last_framerate_get_ms != 0 && now_ms > timestamp_of_last_framerate_get_ms)
    {
        unsigned int localFrameCount = frame_count;
        result = (localFrameCount - last_reported_frame_count) * 1000 / float(now_ms - timestamp_of_last_framerate_get_ms);
        last_reported_frame_count = localFrameCount;
    }
    timestamp_of_last_framerate_get_ms = now_ms;
    return result;
}

void AVDecoder::set_keyframe_request_callback(IRequest_Video_Keyframe *_keyframeRequestCallback)
{
    keyframe_request_callback = _keyframeRequestCallback;
}

void AVDecoder::try_requesting_keyframe()
{
    if(keyframe_request_callback)
        keyframe_request_callback->try_requesting_video_keyframe();

    realtimeStream->send_rtcp_fir(ssrc);
}
