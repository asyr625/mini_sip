#include "reliable_media_server.h"


Reliable_Media_Server::Reliable_Media_Server( Mini_Sip* _minisip, std::string sdpType)
    : Media(_minisip),
      sdp_type(sdpType)
{
}

Reliable_Media_Server::~Reliable_Media_Server()
{
}
