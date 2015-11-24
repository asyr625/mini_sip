#include "reliable_media.h"


Reliable_Media::Reliable_Media( Mini_Sip* _minisip, std::string sdptype, bool isClient, bool isServer )
    : Media(_minisip),
      sdp_type(sdptype)
{
}

Reliable_Media::~Reliable_Media()
{
}

std::string Reliable_Media::get_sdp_media_type()
{
    return sdp_type;
}
