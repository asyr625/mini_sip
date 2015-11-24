#ifndef SOUND_SOURCE_H
#define SOUND_SOURCE_H
#include <string>

#include "mutex.h"
#include "resampler.h"
#include "sp_audio.h"
#include "sound_io_plc_interface.h"

#define LEFT 1
#define RIGHT 5
#define CENTER 3

class Circular_Buffer;

class Sound_Source : public virtual SObject
{
public:
    Sound_Source(int id, std::string callId);

    virtual ~Sound_Source();

    virtual std::string get_mem_object_type() const {return "SoundSource"; }

    int get_id();

    std::string get_call_id(){return callid;}

    int32_t get_pos();

    void set_pos(int32_t p);

    virtual void push_sound(short *samples, int32_t nSamples, int32_t index, int freq, bool isStereo /* = false*/) = 0;

    virtual void get_sound(short *dest, bool dequeue = true) = 0;

    bool is_silenced() { return silenced; }
    void set_silenced( bool s ) { silenced = s; }

    int32_t get_pointer();
    void set_pointer(int32_t wpointer);

protected:
    int source_id;
    volatile bool silenced;
    int32_t position;
    short *leftch;
    short *rightch;
    int32_t pointer;
    int32_t j;
    int32_t k;
    friend class Sp_Audio;

    std::string callid;
    uint64_t of_error_timestamp;
    uint64_t uf_error_timestamp;
    int uf_error_count;
    int of_error_count;
};

class Basic_Sound_Source : public Sound_Source
{
public:
    Basic_Sound_Source(int32_t id, std::string callId, Sound_IO_PLC_Interface *plc,
        int32_t position, uint32_t oFreq, uint32_t oDurationMs, uint32_t oNChannels);

    virtual ~Basic_Sound_Source();

    void push_sound(short *samples, int32_t nMonoSamples, int32_t index, int samplerate, bool isStereo = false);

    virtual void get_sound(short *dest, bool dequeue = true);

private:
    Sound_IO_PLC_Interface *plc_provider;

    Circular_Buffer * cbuff;

    short plc_cache[2048];

    int ofreq;

    short *temp;
    Mutex buffer_lock;
    uint32_t oframes;
    uint32_t iframes;

    uint32_t onchannels;
    SRef<Resampler *> resampler;
};

#endif // SOUND_SOURCE_H
