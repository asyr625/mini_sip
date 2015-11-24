#ifndef MIKEY_H
#define MIKEY_H
#include <vector>

#include "sobject.h"
#include "key_agreement.h"

class Sip_Sim;

class IMikey_Config : public virtual SObject
{
public:
    virtual ~IMikey_Config();

    virtual const std::string get_uri() const = 0;

    virtual SRef<Sip_Sim*> get_sim() const = 0;

    virtual size_t get_psk_length() const = 0;
    virtual const byte_t* get_psk() const = 0;

    virtual bool is_method_enabled( int kaType ) const = 0;

    virtual bool is_cert_check_enabled() const = 0;
};

class Mikey : public SObject
{
public:
    enum State
    {
        STATE_START = 0,
        STATE_INITIATOR,
        STATE_RESPONDER,
        STATE_AUTHENTICATED,
        STATE_ERROR
    };

    typedef std::vector<uint32_t> Streams;

    Mikey(SRef<IMikey_Config*> _config );
    ~Mikey();

    std::string initiator_create( int kaType, const std::string &peerUri="" );
    bool initiator_authenticate( std::string message );
    std::string initiator_parse();

    bool responder_authenticate( const std::string &message, const std::string &peerUri="" );
    std::string responder_parse();

    void set_mikey_offer();

    bool is_secured() const;
    bool is_initiator() const;
    bool error() const;
    std::string auth_error() const;
    SRef<Key_Agreement*> get_key_agreement() const;

    void add_sender( uint32_t ssrc );

    const std::string &peer_uri() const;

protected:
    void set_state( State newState );

private:
    void create_key_agreement( int type );
    void add_streams_to_ka();

    State state;
    bool secured;
    SRef<IMikey_Config*> config;
    Streams media_stream_senders;
    SRef<Key_Agreement *> ka;
};

#endif // MIKEY_H
