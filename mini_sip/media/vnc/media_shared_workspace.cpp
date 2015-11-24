
#include "media_shared_workspace.h"
#include "media_stream.h"

using namespace std;

class SWS_Media_Stream : public Reliable_Media_Stream
{
public:
    SWS_Media_Stream( std::string callId, SRef<Reliable_Media*> m );
    std::string get_media_formats();

	void start();
	void stop();
    virtual uint16_t get_port();
    virtual uint16_t get_port(string transport);
};

SWS_Media_Stream::SWS_Media_Stream( std::string callId, SRef<Reliable_Media*> m ) : Reliable_Media_Stream(callId,m, NULL)
{
}

string SWS_Media_Stream::get_media_formats()
{
	return "vnc";
}

void SWS_Media_Stream::start()
{
}

void SWS_Media_Stream::stop()
{
}

uint16_t SWS_Media_Stream::get_port()
{
	return 0;
}

uint16_t SWS_Media_Stream::get_port(string transport)
{
	return 0;
}


Media_Shared_Workspace::Media_Shared_Workspace(Mini_Sip* minisip) : Reliable_Media(minisip, "application", true,true)
{
    add_sdp_attribute("recvonly");
    add_sdp_attribute("setup:active");
}

string Media_Shared_Workspace::get_sdp_media_type()
{
	return "application";
}

SRef<Reliable_Media_Stream*> Media_Shared_Workspace::create_media_stream(string callId)
{
    SRef<Reliable_Media_Stream*> m =  new SWS_Media_Stream(callId, this);
	return m;
}


uint8_t Media_Shared_Workspace::get_codec_get_sdp_media_type()
{
    return 0;
}

