#ifndef RELIABLE_MEDIA_H
#define RELIABLE_MEDIA_H

#include "media.h"

class Reliable_Media_Stream;

class Reliable_Media : public Media
{
public:
    ~Reliable_Media();

    virtual SRef<Reliable_Media_Stream*> create_media_stream(std::string callId) = 0;

    virtual std::string get_sdp_media_type();

protected:
    Reliable_Media(Mini_Sip* _minisip, std::string sdptype, bool isClient, bool isServer );

    std::string sdp_type;
};

#endif // RELIABLE_MEDIA_H
