#ifndef SOUND_RECORDER_CALLBACK_H
#define SOUND_RECORDER_CALLBACK_H

class Sound_Recorder_Callback
{
public:
    virtual ~Sound_Recorder_Callback() {}
    virtual void srcb_handle_sound(void *samplearr, int length, int sample_freq) = 0;
#ifdef AEC_SUPPORT
    virtual void srcb_handle_sound(void *samplearr, int length, void *samplearrR) = 0;		//hanning
#endif
};

#endif // SOUND_RECORDER_CALLBACK_H
