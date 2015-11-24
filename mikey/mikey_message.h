#ifndef MIKEY_MESSAGE_H
#define MIKEY_MESSAGE_H

#include "mikey_defs.h"

#include<assert.h>
#include<list>
#include<iostream>

#include "mikey_payload.h"
#include "mikey_payload_sign.h"
#include "key_agreement.h"


//#include<libmikey/Key_Agreement_DH.h>
//#include<libmikey/Key_Agreement_PSK.h>
#include "cert.h"

#define MIKEY_TYPE_PSK_INIT    0
#define MIKEY_TYPE_PSK_RESP    1
#define MIKEY_TYPE_PK_INIT     2
#define MIKEY_TYPE_PK_RESP     3
#define MIKEY_TYPE_DH_INIT     4
#define MIKEY_TYPE_DH_RESP     5
#define MIKEY_TYPE_ERROR       6
#define MIKEY_TYPE_DHHMAC_INIT 7
#define MIKEY_TYPE_DHHMAC_RESP 8
#define MIKEY_TYPE_RSA_R_INIT  9
#define MIKEY_TYPE_RSA_R_RESP 10

#define MIKEY_ENCR_NULL       0
#define MIKEY_ENCR_AES_CM_128 1
#define MIKEY_ENCR_AES_KW_128 2

#define MIKEY_MAC_NULL          0
#define MIKEY_MAC_HMAC_SHA1_160 1

#define MAX_TIME_OFFSET (int64_t)(0xe100000<<16) //1 hour

class aes;
class Sip_Sim;
class Certificate;
class Certificate_Set;
class Key_Agreement;
class Key_Agreement_DH;
class Key_Agreement_DHHMAC;
class Key_Agreement_PKE;
class Key_Agreement_PSK;
class Key_Agreement_RSAR;
class Mikey_Payload_ID;
class Mikey_Message;

class Mikey_Payloads : public SObject
{
public:
    Mikey_Payloads();
    Mikey_Payloads(int firstPayloadType, byte_t *message, int lengthLimit );
    virtual ~Mikey_Payloads();

    void add_payload( SRef<Mikey_Payload*> payload );
    void operator+=( SRef<Mikey_Payload*> payload );

    void add_signature_payload(SRef<Sip_Sim*> sim, bool addIdsAndT = false );
    void add_signature_payload(SRef<Certificate *> cert, bool addIdsAndT = false );
    bool verify_signature(SRef<Certificate*> cert, bool addIdsAndT = false );

    void add_vpayload(int macAlg, uint64_t t, byte_t * authKey, uint32_t authKeyLength);
    bool verifyV( Key_Agreement_PSK* ka );

    void add_kemac_payload( byte_t * tgk, int tgkLength, byte_t * encrKey, byte_t * iv, byte_t * authKey,
                          int encrAlg, int macAlg, bool kemacOnly = false );
    bool verify_kemac( Key_Agreement_PSK* ka, bool kemacOnly = false );

    void add_certificate_payloads( SRef<Certificate_Chain *> certChain );
    SRef<Certificate_Chain*> extract_certificate_chain() const;

    void add_pke_kemac( Key_Agreement_PKE* ka, int encrAlg, int macAlg );
    bool extract_pke_env_key( Key_Agreement_PKE* ka ) const;

    void add_id(const std::string &theId );
    const Mikey_Payload_ID* extract_id( int index ) const;
    std::string extract_id_str( int index ) const;
    std::vector<byte_t> extract_id_vec( int index ) const;

    std::string debug_dump();
    byte_t * raw_message_data();
    int raw_message_length();

    std::list<SRef<Mikey_Payload*> >::const_iterator first_payload() const;
    std::list<SRef<Mikey_Payload*> >::const_iterator last_payload() const;

    std::list<SRef<Mikey_Payload*> >::iterator first_payload();
    std::list<SRef<Mikey_Payload*> >::iterator last_payload();

    SRef<Mikey_Payload*> extract_payload( int type );
    SRef<const Mikey_Payload*> extract_payload(int payloadType ) const;
    void remove( SRef<Mikey_Payload*> );

    std::string b64_message();

protected:
    static void parse( int first_payloadType, byte_t *message, int lengthLimit, std::list<SRef<Mikey_Payload*> >& payloads);

    void add_policy_to_payload(Key_Agreement * ka);
    void add_policy_to_ka(Key_Agreement * ka);
    std::vector<byte_t> build_sign_data(size_t sigLength, bool useIdsT = false );

    void set_raw_message_data( byte_t *data );

    bool verify_mac( Key_Agreement_PSK* ka, int macAlg, const byte_t* receivedMac, const byte_t* macInput, unsigned int macInputLength ) const;

    bool derive_transp_keys( Key_Agreement_PSK* ka, byte_t*& encrKey, byte_t *& iv, unsigned int& encrKeyLength,
                   int encrAlg, int macAlg, uint64_t t, Mikey_Message* errorMessage );

    std::list<SRef<Mikey_Payload*> > payloads;

private:
    void compile();
    bool compiled;
    byte_t *rawData;
};

class Mikey_Message : public Mikey_Payloads
{
public:
    Mikey_Message();
    static Mikey_Message* create( Key_Agreement_DH * ka );
    static Mikey_Message* create( Key_Agreement_DHHMAC * ka, int macAlg = MIKEY_MAC_HMAC_SHA1_160);
    static Mikey_Message* create( Key_Agreement_PSK * ka, int encrAlg = MIKEY_ENCR_AES_CM_128, int macAlg  = MIKEY_MAC_HMAC_SHA1_160 );

    static Mikey_Message* create(Key_Agreement_PKE* ka, int encrAlg = MIKEY_ENCR_AES_CM_128, int macAlg = MIKEY_MAC_HMAC_SHA1_160 );
    static Mikey_Message* create(Key_Agreement_RSAR* ka);

    static Mikey_Message* parse( byte_t *message, int lengthLimit );
    static Mikey_Message* parse( std::string b64Message );

    virtual ~Mikey_Message();

    int type() const;
    uint32_t csb_id();

    virtual SRef<Mikey_Message *> parse_response( Key_Agreement  * ka );
    virtual void set_offer( Key_Agreement * ka );
    virtual SRef<Mikey_Message *> build_response( Key_Agreement * ka );
    virtual bool authenticate( Key_Agreement  * ka );

    virtual bool is_initiator_message() const;
    virtual bool is_responder_message() const;
    virtual int32_t key_agreement_type() const;
};

#endif // MIKEY_MESSAGE_H
