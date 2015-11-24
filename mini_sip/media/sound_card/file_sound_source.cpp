#include "file_sound_source.h"

#include <fstream>
#include <string.h>
#include <iostream>

using namespace std;

File_Sound_Source::File_Sound_Source( std::string callId, std::string filename, uint32_t id,
                                      uint32_t inputFreq,
                                      uint32_t inputNChannels,
                                      uint32_t outputFreq,
                                      uint32_t outputDurationMs,
                                      uint32_t outputNChannels,
                                      bool rep)
    : Sound_Source( id, callId ),
      enabled(false),
      repeat(rep),
      index(0)
{
    short * input;
    long l,m;

#ifdef LIBMINISIP_DATADIR
    if (filename.size()>0 && filename[0]!='/')
    {
        string datadir = LIBMINISIP_DATADIR;
        if (datadir[datadir.size()-1]!='/')
            datadir = datadir+"/";
        datadir = datadir + "libminisip/";
        filename = datadir + filename;
    }
#endif

    ifstream file (filename.c_str(), ios::in|ios::binary);

    l = file.tellg();
    file.seekg (0, ios::end);
    m = file.tellg();
#define WAV_HEADER_SIZE 44
    m -= WAV_HEADER_SIZE; //wave header size

    if(l>=0 && m>=0)
    {
        nsamples = (m-l)/(sizeof(short)*inputNChannels);
        cerr << "File_Sound_Source: nSample: " << nsamples << endl;
    }
    else
    {
        nSamples = 0;
        cerr << "File_Sound_Source error, unable to read the file " << filename.c_str() << endl;
    }

    noutput_frames = ( outputDurationMs * inputFreq ) / 1000;
    noutput_frames_resampled = ( outputDurationMs * outputFreq ) / 1000;

    cerr << "File_Sound_Source: noutput_frames: " << noutput_frames << endl;

    input = new short[nsamples * inputNChannels];

    file.seekg (WAV_HEADER_SIZE, ios::beg);
    file.read( (char*)input, nsamples*sizeof(short));

    if( inputNChannels > outputNChannels )
    {
        audio = new short[nsamples*outputNChannels];
        if( inputNChannels % outputNChannels == 0 )
        {
            /* do some mixing */
            for( uint32_t sample = 0; sample < nsamples; sample++ )
            {
                for( uint32_t oChannel = 0; oChannel < outputNChannels; oChannel++ )
                {
                    for( uint32_t i = 0; i < inputNChannels/outputNChannels; i++ )
                    {
                        audio[sample*outputNChannels+oChannel] += input[sample*inputNChannels + oChannel + i*inputNChannels];
                    }
                    audio[sample*outputNChannels+oChannel] /= inputNChannels/outputNChannels;
                }
            }
        }
        else
        {
            /* Just take the first channels */
            for( uint32_t sample = 0; sample < nsamples; sample++ )
            {
                for( uint32_t oChannel = 0; oChannel < outputNChannels; oChannel++ )
                {
                    audio[sample*outputNChannels+oChannel] = input[sample*inputNChannels+oChannel];
                }
            }
        }
        delete [] input;
    }
    else if( inputNChannels == outputNChannels )
    {
        audio = input;
    }
    else
    {
        audio = new short[nsamples*outputNChannels];
        for( uint32_t sample = 0; sample < nsamples; sample++ )
        {
            for( uint32_t oChannel = 0; oChannel < outputNChannels; oChannel++ )
            {
                audio[sample*outputNChannels+oChannel] = input[sample*inputNChannels];
            }
        }
        delete [] input;
    }

    nchannels = outputNChannels;

    resampler = Resampler_Registry::get_instance()->create( inputFreq, outputFreq, outputDurationMs, outputNChannels );
}

File_Sound_Source::File_Sound_Source(std::string callId, short *raw_audio, int samples, bool repeat)
    : Sound_Source(0,callid),
      audio(rawaudio),
      nsamples(samples),
      enabled(false),
      repeat(rep),
      index(0)
{
}

File_Sound_Source::~File_Sound_Source()
{
    delete [] audio;
    audio = NULL;
}

void File_Sound_Source::enable()
{
    enabled = true;
    index = 0;
}

void File_Sound_Source::disable()
{
    enabled = false;
    index = 0;
}

void File_Sound_Source::push_sound(short *, int32_t , int32_t , int , bool )
{
#ifdef DEBUG_OUTPUT
    cerr << "WARNING: FileSoundSource::push_sound: FORBIDDEN"<< endl;
#endif
}

void File_Sound_Source::get_sound(short *dest, bool dequeue)
{
    if( index + noutput_frames >= nsamples )
    {
        if (repeat)
            index = 0;
        else
            index = nsamples;
    }

    if( (uint32_t)index == nsamples )
    {
        memset( dest, '\0', noutput_frames_resampled * nchannels * sizeof(short) );
    }
    else
    {
        resampler->resample( audio + index, dest );
        //cerr << "audio + index: "  << print_hex( (unsigned char *)(audio + index), noutput_frames*nchannels) << endl;
        //cerr << "dest: "  << print_hex( (unsigned char *)dest, noutput_frames*nchannels) << endl;

        if (dequeue)
        {
            index += noutput_frames*sizeof(short);
        }
    }
}
