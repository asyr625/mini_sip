#ifndef KEY_AGREEMENT_DH_H
#define KEY_AGREEMENT_DH_H

#include "sip_sim.h"
#include "key_agreement.h"

#define DH_GROUP_OAKLEY5 0
#define DH_GROUP_OAKLEY1 1
#define DH_GROUP_OAKLEY2 2


class OakleyDH;
class Certificate_Chain;
class certificate;
class Certificate_Set;

#ifdef _MSC_VER
#pragma warning (disable: 4250)
#endif

class Peer_Certificates
{
public:
    Peer_Certificates( SRef<Certificate_Chain*> aCert, SRef<Certificate_Set *> aCaDb );
    Peer_Certificates( SRef<Certificate_Chain*> aCert, SRef<Certificate_Chain*> aPeerCert );
    virtual ~Peer_Certificates();
    virtual SRef<Certificate_Chain *> certificate_chain();
    virtual SRef<Certificate_Chain *> peer_certificate_chain();
    virtual void set_peer_certificate_chain( SRef<Certificate_Chain *> chain );
    virtual int control_peer_certificate( const std::string &peerUri );

private:
    SRef<Certificate_Chain *> cert_chain_ptr;
    SRef<Certificate_Chain *> peer_cert_chain_ptr;
    SRef<Certificate_Set *> cert_db_ptr;
};

class Key_Agreement_DH_Base : virtual public ITgk
{
public:
    Key_Agreement_DH_Base(SRef<Sip_Sim* > s);
    ~Key_Agreement_DH_Base();

    int compute_tgk();
    int set_group(int groupValue );
    int group();

    void set_peer_key( byte_t * peerKey, int peerKeyLength );
    int peer_key_length();
    byte_t * peer_key();

    int public_key_length();
    byte_t * public_key();

private:
    SRef<Sip_Sim*> sim;
    OakleyDH * dh;
    byte_t * peer_key_ptr;
    int peer_key_length_value;
    byte_t * public_key_ptr;
    int public_key_length_value;
};

class Key_Agreement_DH : public Key_Agreement, public Peer_Certificates, public Key_Agreement_DH_Base
{
public:
    Key_Agreement_DH( SRef<Certificate_Chain *> cert, SRef<Certificate_Set *> certificate_set );
    Key_Agreement_DH( SRef<Sip_Sim *> sim );
    ~Key_Agreement_DH();

    int32_t type();

    Mikey_Message* create_message();
};

#endif // KEY_AGREEMENT_DH_H
