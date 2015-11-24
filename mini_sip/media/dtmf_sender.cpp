#include <list>
#include "dtmf_sender.h"
#include "session.h"

Dtmf_Sender::Dtmf_Sender( SRef<Session *> session_ )
{
    this->session = session_;
}

void Dtmf_Sender::timeout( Dtmf_Event * event )
{
    uint8_t payload[4];
    payload[0] = event->symbol;
    payload[1] = ( (event->endOfEvent << 7) & 0x80 ) | ( event->volume & 0x3F );
    payload[2] = (event->duration >> 8) & 0xFF;
    payload[3] = event->duration & 0xFF;

    send_payload( payload, event->startOfEvent, event->ts );

    if( event->lastBlock )
    {
        delete event->ts;
    }
    delete event;
}

void Dtmf_Sender::send_payload( uint8_t payload[], bool mark, uint32_t * ts )
{
    std::list<SRef<Realtime_Media_Stream_Sender *> >::iterator iSender;

    session->realtime_media_stream_senders_lock.lock();
    for( iSender =  session->realtime_media_stream_senders.begin(); iSender != session->realtime_media_stream_senders.end();
         iSender++ )
    {
        if( !(**iSender)->_disabled )
        {
            ((Realtime_Media_Stream_Sender *)(**iSender))->send( payload, 4, ts, mark, true );
        }
    }
    session->realtime_media_stream_senders_lock.unlock();
}
