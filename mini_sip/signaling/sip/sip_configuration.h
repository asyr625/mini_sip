#ifndef SIP_CONFIGURATION_H
#define SIP_CONFIGURATION_H

#include "sobject.h"

#include "phone_book.h"
#include "contact_db.h"

#include "sip_dialog_config.h"

#include "conf_backend.h"

class Sip_Configuration : public SObject
{
public:
    Sip_Configuration();
    virtual ~Sip_Configuration();

    virtual std::string get_mem_object_type() const {return "SipSoftPhoneConfig";}

    void log_configuration_details(SRef<Sip_Configuration *> phoneConf);
    void save();
    std::string load( SRef<Conf_Backend *> be );

    static std::string get_default_phone_book_filename();

    static void install_config_file(std::string config, std::string address="", bool overwrite=false);

    bool check_version( uint32_t fileVersion );

    void save_default( SRef<Conf_Backend *> backend );

    SRef<Sip_Stack_Config *> _sip_stack_config;

    SRef<Sip_Stack*> _sip_stack;

    SRef<Sip_Identity *> _pstn_identity;
    SRef<Sip_Identity *> _default_identity;

    std::list< SRef<Sip_Identity*> > _identities;

    SRef<Sip_Identity*> get_identity( std::string id );

    SRef<Sip_Identity *> get_identity( const Sip_Uri &uri );

    bool _use_stun;
    std::string _stun_server_ip_string;
    uint16_t _stun_server_port;

    bool _find_stun_server_from_sip_uri;
    bool _find_stun_server_from_domain;
    std::string _stun_domain;

    bool _use_user_defined_stun_server;
    std::string _user_defined_stun_server;

    bool _use_anat;
    bool _use_ipv6;
    typedef enum { luckyGuess, ipv4only, ipv6only, autodetect } IPStack;
    IPStack ipStack;
    unsigned char _video_stream_ipv6_dscp, _audio_stream_ipv6_dscp;

    std::string _sound_device_in;
    std::string _sound_device_out;
    std::string _video_device;
    std::string _video_device2;
    struct videoEncoder {
        unsigned int width, height, framerate, bitrate;
    } video_encoder1, video_encoder2;
    std::string _display_frame_size;
    std::string _display_frame_rate;

    void decode_video_encoder_format(const std::string &videoEncoderString, videoEncoder& videoEncoder);

    uint32_t _frame_width;
    uint32_t _frame_height;

    bool _use_pstn_proxy;

    std::list<SRef<Phone_Book *> > _phonebooks;
    Mutex _phonebooks_mutex;

    std::string _ringtone;

    int _rtp_port_range_start;
    int _rtp_port_range_end;

    std::string _log_server_addr;
    std::string _log_server_port;

    bool _logging_flag;
    bool _local_logging_flag;

    std::list<std::string> _audio_codecs;

    std::string _sound_iomixer_type;

    std::list<std::string> _startup_actions;

    std::string _network_interface_name;

    std::string _sdes_location;

    uint32_t _decoration_color;

    int _video_transparency_percentage;
    int _background_animate;
    std::string _wallpapers;

private:
    SRef<Conf_Backend *> _backend;

    void add_missing_audio_codecs( SRef<Conf_Backend *> be );
};

#endif // SIP_CONFIGURATION_H
