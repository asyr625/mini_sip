#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<sys/mman.h>
#include<errno.h>
#include<unistd.h>

#include<iostream>
#include<fstream>
#include<string.h>
using namespace std;

#include<turbojpeg.h>

#include "v4l2_grabber.h"

#include "video_media.h"
#include "video_exception.h"

#include "dbg.h"
#include "my_time.h"
#include "my_error.h"

/*
  TODO
  The V4L2Grabber contains a unresolved race condition that occurs
  if it's closed before the initialization has finished.
  It is triggered if the peer answers a video call and directly terminates it,
  which results in a crash.
 */

/*RFC2035 start*/

/*
 * Table K.1 from JPEG spec.
 */
static const int jpeg_luma_quantizer[64] =
{
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};

/*
 * Table K.2 from JPEG spec.
 */
static const int jpeg_chroma_quantizer[64] =
{
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

/*
 * Call make_tables with the Q factor and two int[64] return arrays
 */
void
make_tables(int q, u_char *lum_q, u_char *chr_q)
{
    int i;
    int factor = q;

    if (q < 1) factor = 1;
    if (q > 99) factor = 99;
    if (q < 50)
        q = 5000 / factor;
    else
        q = 200 - factor*2;

    for (i=0; i < 64; i++)
    {
        int lq = ( jpeg_luma_quantizer[i] * q + 50) / 100;
        int cq = ( jpeg_chroma_quantizer[i] * q + 50) / 100;

        /* Limit the quantizers to 1 <= q <= 255 */
        if ( lq < 1) lq = 1;
        else if ( lq > 255) lq = 255;
        lum_q[i] = lq;

        if ( cq < 1) cq = 1;
        else if ( cq > 255) cq = 255;
        chr_q[i] = cq;
    }
}

u_char lum_dc_codelens[] =
{
    0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};

u_char lum_dc_symbols[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};

u_char lum_ac_codelens[] =
{
    0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d,
};

u_char lum_ac_symbols[] =
{
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa,
};

u_char chm_dc_codelens[] =
{
    0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
};

u_char chm_dc_symbols[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};

u_char chm_ac_codelens[] =
{
    0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77,
};


u_char chm_ac_symbols[] =
{
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa,
};


u_char * make_quant_header(u_char *p, u_char *qt, int tableNo)
{
    *p++ = 0xff;
    *p++ = 0xdb;            /* DQT */
    *p++ = 0;               /* length msb */
    *p++ = 67;              /* length lsb */
    *p++ = tableNo;
    memcpy(p, qt, 64);
    return (p + 64);
}

u_char *make_huffman_header(u_char *p, u_char *codelens, int ncodes, u_char *symbols,
                            int nsymbols, int tableNo, int tableClass)
{
    *p++ = 0xff;
    *p++ = 0xc4;            /* DHT */
    *p++ = 0;               /* length msb */
    *p++ = 3 + ncodes + nsymbols; /* length lsb */
    *p++ = tableClass << 4 | tableNo;
    memcpy(p, codelens, ncodes);
    p += ncodes;
    memcpy(p, symbols, nsymbols);
    p += nsymbols;
    return (p);
}


/*
 * Given an RTP/JPEG type code, q factor, width, and height,
 * generate a frame and scan headers that can be prepended
 * to the RTP/JPEG data payload to produce a JPEG compressed
 * image in interchange format (except for possible trailing
 * garbage and absence of an EOI marker to terminate the scan).
 */
int make_headers(u_char *p, int type, int q, int w, int h)
{
    u_char *start = p;
    u_char lqt[64];
    u_char cqt[64];

    /* convert from blocks to pixels */
    w <<= 3;
    h <<= 3;

    make_tables(q, lqt, cqt);

    *p++ = 0xff;
    *p++ = 0xd8;            /* SOI */

    p = make_quant_header(p, lqt, 0);

    p = make_quant_header(p, cqt, 1);

    p = make_huffman_header(p, lum_dc_codelens,
                            sizeof(lum_dc_codelens),
                            lum_dc_symbols,
                            sizeof(lum_dc_symbols), 0, 0);
    p = make_huffman_header(p, lum_ac_codelens,
                            sizeof(lum_ac_codelens),
                            lum_ac_symbols,
                            sizeof(lum_ac_symbols), 0, 1);
    p = make_huffman_header(p, chm_dc_codelens,
                            sizeof(chm_dc_codelens),
                            chm_dc_symbols,
                            sizeof(chm_dc_symbols), 1, 0);
    p = make_huffman_header(p, chm_ac_codelens,
                            sizeof(chm_ac_codelens),
                            chm_ac_symbols,
                            sizeof(chm_ac_symbols), 1, 1);
    return (p - start);
}

/*RFC2035 end*/

#define CLEAR(x) memset (&(x), 0, sizeof (x))

static int xioctl (int fd, int request, void * arg)
{
    int r;
    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

static void errno_exit(const char * s)
{
    fprintf (stderr, "%s error %d, %s\n",
             s, errno, strerror (errno));
    exit (EXIT_FAILURE);
}


//example of device: /dev/video1,800x600@30
V4l2_Grabber::V4l2_Grabber( SRef<Video_Media*> _media, std::string _device )
    : Grabber(_media)
{
    stopped = false;
    fd = -1;
    buffers = NULL;
    n_buffers = 0;
    grab_width = grab_height = 0;

    requested_width = 1280; //set default values
    requested_height = 720;
    requested_fps = 30;

    int position;
    if ((position = _device.find(',')) == string::npos)
        device = _device;
    else
    {
        device = _device.substr(0,position);
        string fmt = _device.substr(position+1);
        if ((position = fmt.find('@')) != string::npos)
        {
            string fps = fmt.substr(position+1);
            fmt = fmt.substr(0,position);
            requested_fps = atoi(fps.c_str());
        }

        if ((position = fmt.find('x')) != string::npos)
        {
            string w = fmt.substr(0,position);
            string h = fmt.substr(position+1);
            requested_width = atoi(w.c_str());
            requested_height = atoi(h.c_str());
        }else{
            my_err <<"WARNING: Capture size not recognized - using default values"<<endl;
        }
    }

    my_dbg << "V4l2_Grabber: device " << device<<" "<<requested_width<<"x"<<requested_height<< "@"<<requested_fps << endl;
}
void V4l2_Grabber::uninit()
{
    unsigned int i;
    for (i = 0; i < n_buffers; ++i)
        if (-1 == munmap (buffers[i].start, buffers[i].length))
            errno_exit ("munmap");
    free (buffers);

    if(local_display)
        local_display->stop();
}

void V4l2_Grabber::init()
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;
    int io = IO_METHOD_MMAP;

    if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno)
        {
            fprintf (stderr, "%s is no V4L2 device\n", device.c_str());
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        fprintf (stderr, "%s is no video capture device\n",
                 device.c_str());
        exit (EXIT_FAILURE);
    }

    switch (io)
    {
    case IO_METHOD_READ:
        if (!(cap.capabilities & V4L2_CAP_READWRITE))
        {
            fprintf (stderr, "%s does not support read i/o\n",
                     device.c_str() );
            exit (EXIT_FAILURE);
        }
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        if (!(cap.capabilities & V4L2_CAP_STREAMING))
        {
            fprintf (stderr, "%s does not support streaming i/o\n",
                     device.c_str() );
            exit (EXIT_FAILURE);
        }
        break;
    }

    /* Select video input, video standard and tune here. */
    CLEAR (cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;


    if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap))
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop))
        {
            switch (errno)
            {
            case EINVAL:
                /* Cropping not supported. */
                break;
            default:
                /* Errors ignored. */
                break;
            }
        }
    } else {
        /* Errors ignored. */
    }

    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index=0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bool supportsYUYV=false;
    bool supportsMJPEG=false;

    while ( xioctl (fd, VIDIOC_ENUM_FMT, &fmtdesc) >=0 )
    {
        if (fmtdesc.pixelformat == V4L2_PIX_FMT_MJPEG)
            supportsMJPEG = true;
        if (fmtdesc.pixelformat == V4L2_PIX_FMT_YUYV)
            supportsYUYV = true;
        fmtdesc.index ++;
    }

    my_assert(supportsMJPEG || supportsYUYV);

    CLEAR (fmt);

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = requested_width;
    fmt.fmt.pix.height      = requested_height;
    if (supportsMJPEG)
    {
        is_mjpeg = true;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    }
    else
    {
        is_mjpeg = false;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    }
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
        errno_exit ("VIDIOC_S_FMT");

    /* Note VIDIOC_S_FMT may change width and height. */

    grab_width = fmt.fmt.pix.width;
    grab_height = fmt.fmt.pix.height;

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

#if 0
    struct v4l2_jpegcompression v4l2_jpeg;
    if (xioctl(fd, VIDIOC_G_JPEGCOMP, &v4l2_jpeg) == -1)
    {
        cerr<<"VIDIOC_G_JPEGCOMP not supported but it should be (does your webcam driver support this ioctl?)"<<endl;
    }
    else
    {
        v4l2_jpeg.jpeg_markers |= V4L2_JPEG_MARKER_DHT;
        if (xioctl(fd, VIDIOC_S_JPEGCOMP, &v4l2_jpeg) == -1)
        {
            cerr<< "VIDIOC_S_JPEGCOMP"<<endl;
        }

    }
#endif

    struct v4l2_streamparm streamparm;
    struct v4l2_fract *tpf;
    memset (&streamparm, 0, sizeof (streamparm));
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tpf = &streamparm.parm.capture.timeperframe;
    tpf->numerator = 1;
    tpf->denominator = requested_fps;
    if (ioctl(fd, VIDIOC_S_PARM, &streamparm) < 0)
    {
        fprintf(stderr,"ERROR: Failed to set camera FPS: %s\n",strerror(errno));
    }

    switch (io)
    {
    case IO_METHOD_MMAP:
        map_memory();
        break;
    default:
        my_assert(0);
    }
}

void V4l2_Grabber::open()
{
    fd = ::open( device.c_str(), O_RDWR | O_NONBLOCK, 0 );
    if( fd == -1 )
    {
        my_error( "open" );
        throw Video_Exception( "Error opening " + device + " " + strerror( errno ) );
    }

    init();
}
void V4l2_Grabber::map_memory()
{
    struct v4l2_requestbuffers req;

    CLEAR (req);

    req.count               = 4;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;

    if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            fprintf (stderr, "%s does not support "
                     "memory mapping\n", device.c_str() );
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2)
    {
        fprintf (stderr, "Insufficient buffer memory on %s\n",
                 device.c_str() );
        exit (EXIT_FAILURE);
    }

    buffers = (cardBuffer*) calloc (req.count, sizeof (*buffers));
    if (!buffers)
    {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
            errno_exit ("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = (uint8_t*)
                mmap (NULL /* start anywhere */,
                      buf.length,
                      PROT_READ | PROT_WRITE /* required */,
                      MAP_SHARED /* recommended */,
                      fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit ("mmap");
    }
}

bool V4l2_Grabber::set_image_size( uint32_t width, uint32_t height )
{
    cerr << "Doing V4l2_Grabber::::set_image_size(): WARNING: size ignored - will try 720p"<<endl;
}

bool V4l2_Grabber::set_image_chroma( uint32_t chroma )
{
    if (chroma == M_CHROMA_I420)
        return true;
    else
        return false;
}

void V4l2_Grabber::start()
{
    my_assert(!runthread); // we don't want to start a second thread
    // for a grabber object
    runthread = new Thread(this, Thread::Normal_Priority);
}

void V4l2_Grabber::stop()
{
    stopped = true;
    if(runthread)
    {
        runthread->join();
        runthread=NULL;
    }
}

void V4l2_Grabber::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("V4L2Grabber::run");
#endif

    stopped = false;
    read();
}


#if 0
void printInfo(uint8_t* buf, int len)
{
    int i = 0;
    cerr<<"EEEE: info for mjpeg data of length: "<<len<<endl;
    while (i<len)
    {
        massert(buf[i]==0xff);
        i++;
        uint8_t  markertype=buf[i++];
        cerr<<"EEEE: marker type="<<hex<<(int) markertype<<dec<<endl;
        int mlen=0;
        switch (markertype){
        case 0xd8:
            cerr<<"EEEE: SOI"<<endl;
            break;
        case 0xC0:
            cerr<<"EEEE: SOF0"<<endl;
            mlen=(buf[i]<<8) | buf[i+1];
            i+=mlen;
            cerr<<"EEEE: segment length: "<<mlen<<endl;
            break;
        case 0xC2:
            cerr<<"EEEE: SOF2"<<endl;
            break;
        case 0xC4:
            cerr<<"EEEE: DHT"<<endl;
            break;
        case 0xDB:
            cerr<<"EEEE: DQT"<<endl;
            mlen=(buf[i]<<8) | buf[i+1];
            i+=mlen;
            cerr<<"EEEE: segment length: "<<mlen<<endl;
            break;

        case 0xDD:
            cerr<<"EEEE: DRI"<<endl;
            i+=4;
            break;
        case 0xDA:
            cerr<<"EEEE: SOS"<<endl;
            //mlen=(buf[i]<<8) | buf[i+1];
            //i+=mlen;
            while (buf[i]!=0xff)
                i++;
            cerr<<"EEEE: segment length: "<<mlen<<endl;
            break;

        case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7: case 0xE8: case 0xE9: case 0xEa: case 0xEb: case 0xEc: case 0xEd: case 0xEe: case 0xEf:
            cerr<<"EEEE: APP specific"<<endl;
            mlen=(buf[i]<<8) | buf[i+1];
            cerr<<"EEEE: segment length: "<<mlen<<endl;
            cerr<<"EEEE: APP DATA: "<< bin_to_hex((unsigned char*)&buf[i+2], mlen-2)<<endl;
            cerr<<"EEEE: as string: "<<&buf[i+2]<<endl;
            i+=mlen;
            break;

        default:
            my_assert(0);
        }
    }
}
#endif

void V4l2_Grabber::read()
{
    if (local_display)
    {
        local_display->set_is_local_video(true);
        local_display->start();
    }

    unsigned int i;
    enum v4l2_buf_type type;

    int io = IO_METHOD_MMAP;

    switch (io)
    {
    case IO_METHOD_MMAP:
        for (i = 0; i < n_buffers; ++i)
        {
            struct v4l2_buffer buf;

            CLEAR (buf);

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = i;

            printf("doing VIDIOC_QBUF\n");
            if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                errno_exit ("VIDIOC_QBUF");
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        printf("doing VIDIOC_STREAMON\n");
        if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
            errno_exit ("VIDIOC_STREAMON");
        break;
    }


    SRef<SImage *> framergb    = SImage::factory(grab_width, grab_height, M_CHROMA_RV24); //if yuyv used for local display, if mjpeg used for decode
    SRef<SImage *> frameyuyv;
    if (!is_mjpeg)
        frameyuyv = SImage::factory(grab_width, grab_height, M_CHROMA_YUYV);

    struct v4l2_buffer buf;

    uint8_t* indata = new uint8_t[1024*1024*3];

    //int type=1;
    int _type = 0;
    int width = grab_width;
    int height = grab_height;
    //int q=90;
    int q = 0;
    uint8_t headers[4096];
    int hlen = make_headers(headers, _type, q, width/8, height/8 );
    tjhandle h = tjInitDecompress();

    while (!stopped)
    {
        fd_set fds;
        struct timeval tv;
        FD_ZERO (&fds);
        FD_SET (fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        int r = select (fd + 1, &fds, NULL, NULL, &tv);
        if (-1 == r)
        {
            if (EINTR == errno)
                continue;

            errno_exit ("select");
        }
        if (0 == r)
        {
            //fprintf (stderr, "select timeout\n");
            //exit (EXIT_FAILURE);
            continue;
        }

        switch (io)
        {
        case IO_METHOD_MMAP:
            CLEAR (buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf))
            {
                switch (errno)
                {
                case EAGAIN:
                    continue;

                case EIO:
                    /* Could ignore EIO, see spec. */

                    /* fall through */

                default:
                    errno_exit ("VIDIOC_DQBUF");
                }
            }

            if (is_mjpeg)
            {
                memcpy(&indata[hlen-2], buffers[buf.index].start, buf.bytesused);
                memcpy(&indata[0], headers, hlen);

                //int ret=tjDecompressToYUV(h,indata,buf.bytesused+hlen-2, outdata,0);
                int ret=tjDecompress2(h,indata,buf.bytesused+hlen-2, framergb->data[0], grab_width, grab_height*3, grab_height, TJPF_RGB,0);
                if (ret)
                {
                    cerr<<"ERROR: V4L2 grabbing: MJPEG decompression error : "<<tjGetErrorStr()<<endl;
                }

                framergb->uTime = utime();
                forward(framergb);      // frame is put through device pipeline, and then forwarded to the encoder
                if (local_display )
                {
                    local_display->handle(framergb);
                }
            }else{ // YUYV

                memcpy(frameyuyv->data[0], buffers[buf.index].start, grab_width*grab_height*2);
                frameyuyv->uTime = utime();
                forward(frameyuyv); // frame is put through device pipeline, and then forwarded to the encoder

                if (local_display )
                {
                    uint8_t* out = framergb->data[0];
                    uint8_t* in = frameyuyv->data[0];
                    for (int w = 0; w < grab_height; w++)
                        for (int h = 0; h < grab_width / 2; h++)
                        {
                            uint8_t y1 = *in++;
                            uint8_t u = *in++;
                            uint8_t y2 = *in++;
                            uint8_t v = *in++;
                            //R1
                            *out++ = y1+1.402*(v-127);
                            //G1
                            *out++ = y1-0.344*(u-127)-0.714*(v-127);
                            //B1
                            *out++ = y1+1.882*(u-127);

                            //R2
                            *out++ = y2+1.402*(v-127);
                            //G2
                            *out++ = y2-0.344*(u-127)-0.714*(v-127);
                            //B2
                            *out++ = y2+1.882*(u-127);
                        }
                    //localDisplay->handle(frame, frame->width, frame->height, M_CHROMA_RV24, true);
                    local_display->handle(framergb);
                }
            }
            if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                errno_exit ("VIDIOC_QBUF");
            break;
        }
    }
    delete [] indata;
}

void V4l2_Grabber::close()
{
    my_dbg << "V4L2Grabber: Close" << endl;
    stop();
    if (fd!=-1)
    {
        grabber_lock.lock();
        uninit();
        unmap_memory();
        ::close( fd );
        fd = -1;
        grabber_lock.unlock();
    }
}

void V4l2_Grabber::set_local_display(SRef<Video_Display*> p)
{
    local_display = p;
}

void V4l2_Grabber::unmap_memory()
{
}

