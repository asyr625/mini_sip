#ifndef SOUND_IO_H
#define SOUND_IO_H

#include <list>
#include <string>

#include "mutex.h"
#include "cond_var.h"

#include "audio_mixer.h"
#include "sound_device.h"
#include "sound_recorder_callback.h"

class Sound_IO_PLC_Interface;
class Sound_Source;

class Recorder_Receiver
{
public:
    Recorder_Receiver(Sound_Recorder_Callback *cb, bool stereo);
    bool get_stereo();
    Sound_Recorder_Callback *get_callback();
private:
    Sound_Recorder_Callback *callback;
    bool stereo;
};

class Sound_IO : public SObject
{
public:
    Sound_IO(SRef<Sound_Device *> inputDevice, SRef<Sound_Device *> outputDevice, std::string mixerType,
             int nChannels=2, int32_t speed=8000, int format=SOUND_S16LE);

    virtual ~Sound_IO();
    void open_playback();
    void open_record();

    void start_record();
    void close_playback();
    void close_record();

    void stop_record();
    void sync();
    void play_testtone(int secs=1);

    static void file_io_timeout(int);

    void register_recorder_receiver(Sound_Recorder_Callback *callback, int32_t nrsamples, bool stereo);

    void unregister_recorder_receiver( Sound_Recorder_Callback *callback );
    void start_recorder();

    void set_recorder_buffer_size(int32_t bs);

    void register_source(SRef<Sound_Source *> source);

    void unregister_source(int sourceId);

    void start_sound_player();

    void read_from_card(short *buf, int32_t n_samples);

    SRef<Sound_Source *> get_sound_source(int32_t id);

    virtual std::string get_mem_object_type() const {return "SoundIO";}
    SRef< Audio_Mixer *> get_mixer();

    bool set_mixer(  std::string type );
private:
    void send_to_card(short *buf, int32_t n_samples);

    void cycle_sound_buffers();

    void init_dsp();
    void init_fileio();

    static void *recorder_loop(void *sc_arg);

    static void *player_loop(void *arg);

    SRef< Audio_Mixer *> mixer;


    std::list<SRef<Sound_Source *> > sources;
    std::list<Recorder_Receiver *> recorder_callbacks;

    volatile int32_t recorder_buffer_size;

    Mutex queue_lock;

    int32_t nchannels;
    int32_t sampling_rate;
    int32_t format;

    bool recording;

    Cond_Var *source_list_cond;

    Cond_Var *recorder_cond;

    SRef<Sound_Device *> sound_dev_in;
    SRef<Sound_Device *> sound_dev_out;
};

#endif // SOUND_IO_H
