#ifndef RELIABLE_MEDIA_SERVER_H
#define RELIABLE_MEDIA_SERVER_H

#include "media.h"

class Reliable_Media_Server : public Media
{
public:
    ~Reliable_Media_Server();

    virtual std::string get_sdp_media_type(){return sdp_type;}

    virtual std::list<std::string> get_sdp_attributes() = 0;

    virtual uint16_t get_port() { return 33; }

protected:
    Reliable_Media_Server( Mini_Sip* _minisip, std::string sdpType);

    std::string sdp_type;
};

#endif // RELIABLE_MEDIA_SERVER_H
