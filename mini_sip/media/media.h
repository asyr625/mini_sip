#ifndef MEDIA_H
#define MEDIA_H
#include <list>

#include "sobject.h"
#include "splugin.h"
#include "ssingleton.h"
#include "my_types.h"

class Sdp_HeaderM;
class Mini_Sip;

class Sip_Configuration;

class Media : public SObject
{
public:
    ~Media();
    virtual std::string get_sdp_media_type() = 0;

    std::list<std::string> get_sdp_attributes();

    void add_sdp_attribute( std::string attribute );

    virtual void handle_mheader( SRef<Sdp_HeaderM *> m );

    virtual uint8_t get_codec_get_sdp_media_type() = 0;
    Mini_Sip* get_mini_sip();

protected:
    Media(Mini_Sip* _minisip);
    Mini_Sip* _mini_sip;

    std::list<std::string> _sdp_attributes;
};

class Streams_Player;

class Media_Plugin : public SPlugin
{
public:
    Media_Plugin(SRef<Library*> lib);
    virtual ~Media_Plugin();

    virtual SRef<Media*> create_media( Mini_Sip*, SRef<Sip_Configuration *> config, Streams_Player *streamsPlayer ) = 0;
    virtual SRef<Media*> create_media2stream ( Mini_Sip*, SRef<Sip_Configuration *> config, Streams_Player *streamsPlayer ) = 0;

    virtual std::string get_plugin_type() const{ return "Media"; }
};

class Media_Registry : public SPlugin_Registry, public SSingleton<Media_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "Media"; }

protected:
    Media_Registry();
private:
    friend class SSingleton<Media_Registry>;
};

#endif // MEDIA_H
