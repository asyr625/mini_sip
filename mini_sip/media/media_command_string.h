#ifndef MEDIA_COMMAND_STRING_H
#define MEDIA_COMMAND_STRING_H
#include <string>

class Media_Command_String
{
public:
    static const std::string start_ringing;
    static const std::string stop_ringing;
    static const std::string session_debug;
    static const std::string set_session_sound_settings;
    static const std::string reload;

    static const std::string start_camera;
    static const std::string start_screen;
    static const std::string stop_camera;
    static const std::string stop_screen;
    static const std::string audio_forwarding_enable;
    static const std::string audio_forwarding_disable;

    static const std::string video_forwarding_enable;
    static const std::string video_forwarding_disable;

    static const std::string send_dtmf;
};

#endif // MEDIA_COMMAND_STRING_H
