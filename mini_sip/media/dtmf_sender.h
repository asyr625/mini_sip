#ifndef DTMF_SENDER_H
#define DTMF_SENDER_H

#include "my_types.h"
#include "sobject.h"

class Session;

class Dtmf_Event
{
public:
    Dtmf_Event( uint8_t symbol_, uint8_t volume_,
                uint16_t duration_,
                bool endOfEvent_, bool startOfEvent_,
                uint32_t * ts_, bool lastBlock_ = false )
        : symbol(symbol_), volume(volume_),
          duration(duration_),
          endOfEvent(endOfEvent_), startOfEvent(startOfEvent_),
          ts(ts_), lastBlock(lastBlock_) { }
private:
    uint8_t symbol;
    uint8_t volume;
    uint16_t duration;
    bool endOfEvent;
    bool startOfEvent;
    uint32_t * ts;
    bool lastBlock;

    friend class Dtmf_Sender;
};

class Dtmf_Sender : public SObject
{
public:
    Dtmf_Sender(SRef<Session *> session_ );
    void timeout( Dtmf_Event * event );
    virtual std::string get_mem_object_type() const { return "DtmfSender"; }

private:
    SRef<Session *> session;
    void send_payload( uint8_t payload[], bool mark, uint32_t * ts );
};

#endif // DTMF_SENDER_H
