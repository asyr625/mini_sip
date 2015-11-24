#include "alsa_sound_device.h"
#include "thread.h"
#include "my_time.h"
#include "dbg.h"

using namespace std;

/**
Note (Cesc)
I have tried and getting alsa to work in non-blocking for a wide range of soundcards
is difficult, very.
When it seems to work, you may start getting errors, or clicks on the audio ...
Only turn non-blocking to true (and play with the settings (buffer size - see the header,
and number of periods and periodsize) if sound using alsa is bad. Good luck!
*/

#define OPEN_ALSA_IN_NON_BLOCKING_MODE false;


Alsa_Sound_Device::Alsa_Sound_Device( std::string device )
    : Sound_Device( device ), period_size(0),
      num_periods(0), buffer_size(0),
      read_handle(NULL), write_handle(NULL)
{
}

int Alsa_Sound_Device::close_playback()
{
    if( !opened_playback )
    {
#ifdef DEBUG_OUTPUT
        cerr << "WARNING: doing close on already "
                "closed sound card (ALSA)"<< endl;
#endif
        return -1;
    }

    if( write_handle != NULL )
    {
        snd_pcm_close( write_handle );
        write_handle = NULL;
    }

    opened_playback = false;
    return 1;
}

int Alsa_Sound_Device::close_record()
{
    if( !opened_record )
    {
        cerr << "WARNING: doing close on already "
                "closed sound card (ALSA)"<< endl;
        return -1;
    }

    if( read_handle != NULL )
    {
        snd_pcm_close( read_handle );
        read_handle = NULL;
    }

    opened_record = false;
    return 1;
}

int Alsa_Sound_Device::calculate_alsa_params(const bool &periodSizePresent,
                                             unsigned long &periodSizeMin,
                                             unsigned long &periodSizeMax,
                                             const bool &periodCountPresent,
                                             uint32_t &periodsMin,
                                             uint32_t &periodsMax,
                                             unsigned long &maxBufferSize)
{
    unsigned long buffer;

    //if it has already been opened ... return old values ...
    //	if( this->period_size != 0 && this->num_periods != 0 ) {
    //		periodSizeMin = this->period_size;
    //		periodsMin = this->num_periods;
    //		return 0;
    //	}

    periodsMin = std::max(periodsMin, uint32_t(2)); //we want at least two periods ...
    unsigned long bufferMin = (unsigned long) periodsMin * periodSizeMin;

    //set the goal for the buffer size between the limits ...
    buffer = (MIN_HW_PO_BUFFER/1000) * (this->sampling_rate/1000); //size in alsa Frames
    if( buffer < bufferMin ) buffer = bufferMin;
    else if( buffer > bufferMax ) buffer = bufferMax;

#ifdef DEBUG_OUTPUT
    printf( "Alsa Calc Values: sampling = %u, period size [%u,%u],\n"
            "                  num periods = [%u, %u], buffer size = [%u, %u]\n", this->sampling_rate/1000,
            (unsigned int)periodSizeMin,
            (unsigned int)periodSizeMax,
            (unsigned int)periodsMin,
            (unsigned int)periodsMax,
            (unsigned int)bufferMin,
            (unsigned int)bufferMax );
    printf( "Alsa Calc: buffer we want is = %d [alsaframes]\n", (int)buffer );
#endif
    // we prefer having buffer period size and the smaller possible period size ..
    num_periods = 2;
    if(periodCountPresent)
        num_periods = std::min(std::max(num_periods, periodsMin), periodsMax);

    period_size = buffer / num_periods;
    buffer = std::min(period_size * num_periods, bufferMax);
    if(periodSizePresent)
    {
        period_size = std::min(std::max(period_size, periodSizeMin), periodSizeMax);
        num_periods = buffer / period_size + (buffer % period_size > 0);
        period_size = buffer / num_periods;
        period_size = std::min(std::max(period_size, periodSizeMin), periodSizeMax);
        if((period_size - periodSizeMin) % 4) { // size = periodSizeMin + n*4
            int multiplier = (period_size - periodSizeMin) / 4;
            period_size = periodSizeMin + (multiplier + 1) * 4;
        }
        buffer = std::min(period_size * num_periods, bufferMax);
    }

    /*
        for(  siz = periodSizeMin; siz <= periodSizeMax; siz+=4 ) {
            tmp = per * siz;
            if( tmp >= buffer ) {
                if(  tmp > bufferMax ) { tmp = bufferMax; }
                found = true;
                buffer = tmp;
                break;
            }
        }//inner loop
        if( found ) break;
    } // outer loop
  */

#ifdef DEBUG_OUTPUT
    cerr << "Alsa - CalculatedParams: period_size = " << period_size << "; num_periods = " << num_periods << "; buffer_size = " << buffer << flush << endl;
#endif
    buffer_size = buffer;
    return 0;
}


int Alsa_Sound_Device::open_playback( int samplingRate, int nChannels, int format )
{
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    lock_open.lock();
    // Play ...

    int openMode = 0;
    bool openNonBlocking = OPEN_ALSA_IN_NON_BLOCKING_MODE;

    if( openNonBlocking )
    {
        sleep_time = 20; //min time between calls ... simulated
        openMode = SND_PCM_NONBLOCK;
#ifdef DEBUG_OUTPUT
        cerr << "ALSA: opening playback in non-blocking mode" << endl;
#endif
    } else {
        openMode = 0;
        sleep_time = 0;
#ifdef DEBUG_OUTPUT
        cerr << "ALSA: opening playback in non-blocking mode" << endl;
#endif
    }

    if (snd_pcm_open(&write_handle, dev.c_str(), SND_PCM_STREAM_PLAYBACK,  openMode ) < 0 )
    {
        cerr << "Could not open ALSA sound card (playback)" << endl;
        exit(-1);
    }

    if (snd_pcm_hw_params_any(write_handle, hwparams) < 0)
    {
        cerr << "Could not get ALSA sound card parameters (playback)" << endl;
        exit(-1);
    }

    if (snd_pcm_hw_params_set_access(write_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
    {
        cerr << "Could not set ALSA mode (playback)" << endl;
        exit(-1);
    }

    if (snd_pcm_hw_params_set_channels(write_handle, hwparams, nChannels)<0)
    {
        cerr << "Cound not configure ALSA (playback) for playout on "<<  nChannels<< endl;
        exit(-1);
    }

    this->nchannels_play = nChannels;

    _snd_pcm_format alsaFormat;
    set_format( format );

    switch( format )
    {
    case SOUND_S16LE:
        alsaFormat = SND_PCM_FORMAT_S16_LE;
        break;
    case SOUND_S16BE:
        alsaFormat = SND_PCM_FORMAT_S16_BE;
        break;
    case SOUND_U16LE:
        alsaFormat = SND_PCM_FORMAT_U16_LE;
        break;
    case SOUND_U16BE:
        alsaFormat = SND_PCM_FORMAT_U16_BE;
        break;
    default:
        cerr << "Unhandled sound format (ALSA) (playback)" << endl;
        exit( -1 );
    }

    if (snd_pcm_hw_params_set_format(write_handle, hwparams, alsaFormat) < 0)
    {
        cerr << "Could not set ALSA format (playback)" << endl;
        exit(-1);
    }

    unsigned int wantedSamplingRate = (unsigned int)samplingRate;

    if( snd_pcm_hw_params_set_rate_near(write_handle, hwparams, (unsigned int *)&samplingRate, NULL) < 0)
    {
        cerr << "Could not set ALSA rate (playback)" << endl;
        exit(-1);
    }

    if( (unsigned int)samplingRate != wantedSamplingRate  )
    {
#ifdef DEBUG_OUTPUT
        cerr << "ALSA (playback): Could not set chosen rate of " << wantedSamplingRate << ", set to "<< samplingRate <<endl;
#endif
    }
    this->sampling_rate = samplingRate;

    unsigned long periodSizeMin = 960; //desired size of one period ... in number of frames
    unsigned long periodSizeMax = 0; //size of one period ... in number of frames
    uint32_t numPeriodsMin = 0;  //desired buffer length in number of periods
    uint32_t numPeriodsMax = 0;  //desired buffer length in number of periods
    /*
   * since sometimes snd_pcm_hw_params_get_buffer_size_max does not modify the value
   * it is supposed to return, though snd_pcm_hw_params_test_buffer_size suggests
   * it should, the max_buffer_size is pre-set to something pretending to be
   * possible
   */
    unsigned long max_buffer_size = 100000; //max buffer size allowed
    bool periodCountPresent, periodSizePresent;

    uint32_t startThreshold = 16;  //number of frames in the buffer so the hw will start processing
    uint32_t minAvailable = 32;   //number of available frames so we can read/write

    int32_t dir;

    //fetch the params ... exit if not able ...
    periodSizePresent = (snd_pcm_hw_params_test_period_size(write_handle, hwparams, 160, 0) == 0);
    if(periodSizePresent)
    {
        if( snd_pcm_hw_params_get_period_size_min (hwparams, &periodSizeMin, &dir) < 0 )
        {
            cerr << "Playback: Could not get ALSA period min size" << endl;
            exit( -1 );
        }
        if( snd_pcm_hw_params_get_period_size_max (hwparams, &periodSizeMax, &dir) < 0 )
        {
            cerr << "Playback: Could not get ALSA period max size" << endl;
            exit( -1 );
        }
    }
    periodCountPresent = (snd_pcm_hw_params_test_periods(write_handle, hwparams, 2, 0) == 0);
    if(periodCountPresent)
    {
        if( snd_pcm_hw_params_get_periods_min (hwparams, &numPeriodsMin, &dir) < 0 )
        {
            cerr << "Playback: Could not get ALSA periods min " << endl;
            exit( -1 );
        }
        if( snd_pcm_hw_params_get_periods_max (hwparams, &numPeriodsMax, &dir) < 0 )
        {
            cerr << "Playback: Could not get ALSA periods max " << endl;
            exit( -1 );
        }
    }
    if( snd_pcm_hw_params_get_buffer_size_max (hwparams, &max_buffer_size) < 0 )
    {
        cerr << "Playback: Could not get ALSA max buffer size " << endl;
        exit( -1 );
    }

    periodSizeMin=320; //EE: FIXME: without this, the period size is calculated to be too short, and
    //           we get underruds all the time. Investigate proper fix.

    if( calculate_alsa_params( periodSizePresent,
                               periodSizeMin,
                               periodSizeMax,
                               periodCountPresent,
                               numPeriodsMin,
                               numPeriodsMax,
                               max_buffer_size)  < 0 )
    {
        cerr << "Playback: Could Not calculate Alsa Params" << endl;
        exit( -1 );
    }

#ifdef DEBUG_OUTPUT
    printf( "ALSA playback: setting values numperiods %d, period Size %d\n", (int)this->num_periods, (int)this->period_size );
#endif
    //Set the calculated params ... they should not give an eror ... still, check ...
    if(snd_pcm_hw_params_set_buffer_size_near(write_handle, hwparams, &buffer_size) < 0)
    {
        cerr << "ALSA playback could not set buffer size" << endl;
        exit(-1);
    }
#ifdef DEBUG_OUTPUT
    cout << "ALSA playback buffer size set to " << buffer_size << std::endl;
#endif
    if(periodCountPresent)
        if( snd_pcm_hw_params_set_periods (write_handle, hwparams, this->num_periods, 0) < 0 )
        {
            cerr << "Could not set ALSA (playback) periods" << endl;
            exit( -1 );
        }
    if(periodSizePresent)
    {
        if( snd_pcm_hw_params_set_period_size_near (write_handle, hwparams, &this->period_size, 0) < 0 )
        {
            cerr << "Could not set ALSA (playback) period size" << endl;
            exit( -1 );
        } else {
#ifdef DEBUG_OUTPUT
            cerr << "ALSA playback period size set to " << this->period_size << endl;
#endif
        }
    }
    if (snd_pcm_hw_params(write_handle, hwparams) < 0)
    {
        cerr << "Could not apply parameters to ALSA (playback) sound card for playout" << endl;
        exit(-1);
    }

    if (snd_pcm_sw_params_current(write_handle, swparams) < 0)
    {
        cerr << "Could not get ALSA software parameters (playback)" << endl;
        exit(-1);
    }

    if (snd_pcm_sw_params_set_start_threshold(write_handle, swparams, startThreshold))
    {
#ifdef DEBUG_OUTPUT
        cerr << "Could not set ALSA start threshold (playback)" << endl;
#endif
        // 		exit(-1);
    }

    /* Disable the XRUN detection */
    if (snd_pcm_sw_params_set_stop_threshold(write_handle, swparams, 0x7FFFFFFF))
    {
        cerr << "Could not set ALSA stop threshold (playback)" << endl;
        exit(-1);
    }

    if (snd_pcm_sw_params_set_avail_min(write_handle, swparams, minAvailable))
    {
#ifdef DEBUG_OUTPUT
        cerr << "Could not set ALSA avail_min (playback)" << endl;
#endif
        // 		exit(-1);
    }

    if (snd_pcm_sw_params(write_handle, swparams) < 0)
    {
        cerr << "Could not apply sw parameters to ALSA sound card (playback)" << endl;
        exit(-1);
    }

    // 	snd_pcm_prepare( write_handle );
#ifdef DEBUG_OUTPUT
    cerr << "ALSA OPENED playback!!" << endl << flush;
#endif
    opened_playback = true;
    lock_open.unlock();
    return 1;
}

int Alsa_Sound_Device::open_record( int samplingRate, int nChannels, int format )
{
    snd_pcm_hw_params_t *hwparams2;
    snd_pcm_sw_params_t *swparams2;

    lock_open.lock();

    snd_pcm_hw_params_alloca(&hwparams2);
    snd_pcm_sw_params_alloca(&swparams2);


    if (snd_pcm_open(&read_handle, dev.c_str(), SND_PCM_STREAM_CAPTURE, 0)<0){
        cerr << "Could not open ALSA sound card for recording" << endl;
        exit(-1);
    }

    if (snd_pcm_hw_params_any(read_handle, hwparams2) < 0) {
        cerr << "Could not get ALSA sound card parameters (record) " << endl;
        exit(-1);
    }

    if (snd_pcm_hw_params_set_access(read_handle, hwparams2, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        cerr << "Could not set ALSA mode (record) " << endl;
        exit(-1);
    }

    if (snd_pcm_hw_params_set_channels(read_handle, hwparams2, nChannels)<0){
        //if desired num of channels fails, try setting to 1 channel
        cerr << "Cound not configure ALSA for recording on "<<  nChannels<< "channels." << endl;
        if( nChannels != 1 ){
            if (snd_pcm_hw_params_set_channels(
                        read_handle, hwparams2, 1)<0){
                // try to fall back on 1 channel, which should work
                // in most cases
                cerr << "Cound not configure ALSA for "
                        "recording on 1 channel." << endl;
                cerr << "Minisip will now exit." << endl;
                exit(-1);
            }
            else{
                this->nChannelsRecord = 1;
            }
        }
        else{
            cerr << "Cound not configure ALSA "
                    "for recording on 1 channel." << endl;
            cerr << "Minisip will now exit." << endl;
            exit(-1);
        }
    }
    else{
        this->nChannelsRecord = nChannels;
    }

    setFormat( format );
    _snd_pcm_format alsaFormat;

    switch( format ){
    case SOUND_S16LE:
        alsaFormat = SND_PCM_FORMAT_S16_LE;
        break;
    case SOUND_S16BE:
        alsaFormat = SND_PCM_FORMAT_S16_BE;
        break;
    case SOUND_U16LE:
        alsaFormat = SND_PCM_FORMAT_U16_LE;
        break;
    case SOUND_U16BE:
        alsaFormat = SND_PCM_FORMAT_U16_BE;
        break;
    default:
        cerr << "ALSA: Unhandled sound format (record) " << endl;
        exit( -1 );
    }

    if (snd_pcm_hw_params_set_format(read_handle, hwparams2,
                                     alsaFormat) < 0) {
        cerr << "Could not set ALSA format (record) " << endl;
        exit(-1);
    }

    /*	if (snd_pcm_hw_params_set_buffer_time(read_handle, hwparams2,
                40000, 0) < 0) {
        cerr << "Could not set ALSA buffer time" << endl;
        exit(-1);
    }
    */
    unsigned int wantedSamplingRate = (unsigned int)samplingRate;

    if( snd_pcm_hw_params_set_rate_near(read_handle, hwparams2, (unsigned int *)&samplingRate, NULL) < 0){
        cerr << "Could not set ALSA rate (record) " << endl;
        exit(-1);
    }

    if( (unsigned int)samplingRate != wantedSamplingRate ){
#ifdef DEBUG_OUTPUT
        cerr << "Could not set chosen (record) rate of " << wantedSamplingRate << ", set to "<< samplingRate <<endl;
#endif
    }
    this->sampling_rate = samplingRate;

    unsigned long periodSizeMin = 960; //desired size of one period ... in number of frames
    unsigned long periodSizeMax = 0; //size of one period ... in number of frames
    uint32_t numPeriodsMin = 0;  //min buffer length in number of periods
    uint32_t numPeriodsMax = 0;  //max buffer length in number of periods
    /*
   * since sometimes snd_pcm_hw_params_get_buffer_size_max does not modify the value
   * it is supposed to return, though snd_pcm_hw_params_test_buffer_size suggests
   * it should, the max_buffer_size is pre-set to something pretending to be
   * possible
   */
    unsigned long max_buffer_size = 100000; //max buffer size allowed;
    bool periodCountPresent, periodSizePresent;

    // 	uint32_t startThreshold = 16;  //number of frames in the buffer so the hw will start processing
    // 	uint32_t minAvailable = 32;   //number of available frames so we can read/write

    int32_t dir;

    //fetch the params ... exit if not able ...
    periodSizePresent = (snd_pcm_hw_params_test_period_size(read_handle, hwparams2, 160, 0) == 0);
    if(periodSizePresent) {
        if( snd_pcm_hw_params_get_period_size_min (hwparams2, &periodSizeMin, &dir) < 0 ) {
            cerr << "Record: Could not get ALSA period min size" << endl;
            exit( -1 );
        }
        if( snd_pcm_hw_params_get_period_size_max (hwparams2, &periodSizeMax, &dir) < 0 ) {
            cerr << "Record: Could not get ALSA period max size" << endl;
            exit( -1 );
        }
    }
    periodCountPresent = (snd_pcm_hw_params_test_periods(read_handle, hwparams2, 2, 0) == 0);
    if(periodCountPresent) {
        if( snd_pcm_hw_params_get_periods_min (hwparams2, &numPeriodsMin, &dir) < 0 ) {
            cerr << "Record: Could not get ALSA periods min " << endl;
            exit( -1 );
        }
        if( snd_pcm_hw_params_get_periods_max (hwparams2, &numPeriodsMax, &dir) < 0 ) {
            cerr << "Record: Could not get ALSA periods max " << endl;
            exit( -1 );
        }
    }
    if( snd_pcm_hw_params_get_buffer_size_max (hwparams2, &max_buffer_size) < 0 )
    {
        cerr << "Record: Could not get ALSA max buffer size " << endl;
        exit( -1 );
    }

    periodSizeMin=320;

    if( calculateAlsaParams( periodSizePresent,
                             periodSizeMin,
                             periodSizeMax,
                             periodCountPresent,
                             numPeriodsMin,
                             numPeriodsMax,
                             max_buffer_size)  < 0 )
    {
        cerr << "Record: Could Not calculate Alsa Params" << endl;
        exit( -1 );
    }

#ifdef DEBUG_OUTPUT
    printf( "ALSA Record: setting values %d, %d\n", (int)this->num_periods, (int)this->period_size );
#endif
    //Set the calculated params ... they should not give an eror ... still, check ...
    if(snd_pcm_hw_params_set_buffer_size_near(read_handle, hwparams2, &buffer_size) < 0)
    {
        cerr << "ALSA record could not set buffer size" << endl;
        exit(-1);
    }
#ifdef DEBUG_OUTPUT
    cout << "ALSA record buffer size set to " << buffer_size << std::endl;
#endif
    if(periodCountPresent)
        if( snd_pcm_hw_params_set_periods (read_handle, hwparams2, this->num_periods, 0) < 0 ) {
            cerr << "Record Could not set ALSA periods" << endl;
            exit( -1 );
        }
    if(periodSizePresent) {
        if( snd_pcm_hw_params_set_period_size_near (read_handle, hwparams2, &this->period_size, 0) < 0 ) {
            cerr << "Record Could not set ALSA period size" << endl;
            exit( -1 );
        }
#ifdef DEBUG_OUTPUT
        else {
            cerr << "Record: alsa period size set to " << this->period_size << endl;
        }
#endif
    }
    if (snd_pcm_hw_params(read_handle, hwparams2) < 0) {
        cerr << "Record Could not apply parameters to ALSA sound card for playout" << endl;
        exit(-1);
    }

    if (snd_pcm_sw_params_current(read_handle, swparams2) < 0) {
        cerr << "Record Could not get ALSA software parameters" << endl;
        exit(-1);
    }

    /* Disable the XRUN detection */
    if (snd_pcm_sw_params_set_stop_threshold(read_handle, swparams2, 0x7FFFFFFF)){
        cerr << "Record Could not set ALSA stop threshold" << endl;
        exit(-1);
    }

    if (snd_pcm_sw_params(read_handle, swparams2) < 0) {
        cerr << "Record Could not apply sw parameters to ALSA sound card" << endl;
        exit(-1);
    }

    //snd_pcm_prepare(write_handle);
    snd_pcm_prepare(read_handle);
    //snd_pcm_start(read_handle);

#ifdef DEBUG_OUTPUT
    cerr << "ALSA OPENED record!!" << endl << flush;
#endif
    opened_record = true;
    lock_open.unlock();
    return 0;
}


//Note: in alsa jargon, nSamples would be named nFrames. An AlsaFrame is made of
//   N samples per each channel, and each sample is X bytes long.
int Alsa_Sound_Device::read_from_device( byte_t * buffer, uint32_t nSamples )
{
    int nSamplesRead = 0;

    if( read_handle == NULL )
    {
        return -EBADF;
    }

    //it reads nSamples and returns the number of samples read ...
    // or directly the negative error code if an error
    nSamplesRead = snd_pcm_readi( read_handle, buffer, nSamples );

    //  printf("snd_pcm_readi returned %d out of %d samples\n", nSamplesRead, nSamples);
    return nSamplesRead;
}

int Alsa_Sound_Device::read_error( int errcode, byte_t * buffer, uint32_t nSamples )
{
    string msg = "";
    bool mustReturn = true;
    switch( errcode )
    {
    case -EAGAIN:
    case -EINTR:
        msg = "REAGAIN";
        mustReturn = false;
        break;
    case -EPIPE:
        msg = "REPIPE";
        if( snd_pcm_prepare( read_handle ) == -1 ) { mustReturn = true;}
        else { mustReturn = false; }
        break;
    default:
        msg = "RERROR";
        break;
    }
#ifdef DEBUG_OUTPUT
    my_err<<msg<<endl;
#endif
    if( mustReturn ) { return -1; }
    else { return 0; }
}

int Alsa_Sound_Device::write_to_device( byte_t * buffer, uint32_t nSamples )
{
    int nSamplesWritten = 0;

    if( write_handle == NULL )
    {
        return -EBADF;
    }

    lock_open.lock();
    nSamplesWritten = snd_pcm_writei( write_handle, buffer, nSamples );
    lock_open.unlock();

    //  printf("snd_pcm_writei taken %d out of %d samples\n", nSamplesWritten, nSamples);
    return nSamplesWritten; //the return is already the samples written or a negative value
}

int Alsa_Sound_Device::write_error( int errcode, byte_t * buffer, uint32_t nSamples )
{
    string msg = "";
    bool mustReturn = true;
    switch( errcode )
    {
    case -EAGAIN:
    case -EINTR:
        msg = "WEAGAIN";
        mustReturn = false;
        break;
    case -EPIPE:
    {
        snd_pcm_state_t state;
        state = snd_pcm_state(write_handle);
        msg = "WEPIPE";
        //if( snd_pcm_prepare( write_handle ) == -1 ) { mustReturn = true;}
        int ret;
        if( (ret=snd_pcm_recover( write_handle, -EPIPE, true )) == -1 ) { mustReturn = true;}
        else { mustReturn = false; }
    }
        break;
    default:
        msg = "WERROR";
        break;
    }
#ifdef DEBUG_OUTPUT
    my_err<<msg<<endl;
#endif
    if( mustReturn ) { return -1; }
    else { return 0; }
}

void Alsa_Sound_Device::sync()
{
    if( snd_pcm_drain( write_handle ) < 0 )
    {
#ifdef DEBUG_OUTPUT
        cerr << "ALSA: Error on pcm_drain" << endl;
#endif
        //exit(-1);
    }
}

