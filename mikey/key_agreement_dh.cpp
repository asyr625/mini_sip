#include "key_agreement_dh.h"
#include "mikey_exception.h"
#include "oakleydh.h"
#include "sip_sim_smart_cardgd.h"
#include <vector>
#include <string.h>
#include <iostream>
#include <algorithm>
using namespace std;

Peer_Certificates::Peer_Certificates( SRef<Certificate_Chain*> aCert, SRef<Certificate_Set *> aCaDb )
    : cert_chain_ptr( aCert ),
      cert_db_ptr( aCaDb )
{
    peer_cert_chain_ptr = Certificate_Chain::create();
}

Peer_Certificates::Peer_Certificates( SRef<Certificate_Chain*> aCert, SRef<Certificate_Chain*> aPeerCert )
    : cert_chain_ptr( aCert ),
      peer_cert_chain_ptr( aPeerCert )
{
}

Peer_Certificates::~Peer_Certificates()
{
}

SRef<Certificate_Chain *> Peer_Certificates::certificate_chain()
{
    return cert_chain_ptr;
}

SRef<Certificate_Chain *> Peer_Certificates::peer_certificate_chain()
{
    return peer_cert_chain_ptr;
}

void Peer_Certificates::set_peer_certificate_chain( SRef<Certificate_Chain *> peerChain )
{
    peer_cert_chain_ptr = peerChain;
}

int Peer_Certificates::control_peer_certificate( const std::string &peerUri )
{
    if( peer_cert_chain_ptr.is_null() || cert_db_ptr.is_null() )
        return 0;

    int res = peer_cert_chain_ptr->control( cert_db_ptr );
    if( !res ){
        return res;
    }

    if( peerUri == "" ){
        return 1;
    }

    SRef<Certificate *> peerCert = peer_cert_chain_ptr->get_first();
    vector<string> altNames;
    altNames = peerCert->get_alt_name( Certificate::SAN_URI );
    if( find( altNames.begin(), altNames.end(), peerUri ) != altNames.end() ){
        return 1;
    }

    string id = peerUri;
    size_t pos = peerUri.find(':');

    if( pos != string::npos ){
        id = peerUri.substr( pos + 1 );
    }

    altNames = peerCert->get_alt_name( Certificate::SAN_RFC822NAME );
    if( find( altNames.begin(), altNames.end(), id ) != altNames.end() ){
        return 1;
    }

    pos = id.find('@');
    if( pos != string::npos ){
        id = id.substr( pos + 1 );
    }

    altNames = peerCert->get_alt_name( Certificate::SAN_DNSNAME );
    if( find( altNames.begin(), altNames.end(), id ) != altNames.end() ){
        return 1;
    }

    cerr << "Peer URI " << peerUri << " not found in subject alt names." << endl;
    return 0;
}


Key_Agreement_DH_Base::Key_Agreement_DH_Base(SRef<Sip_Sim* > s)
    :sim(s),
      dh(NULL),
      peer_key_ptr( NULL ),
      peer_key_length_value( 0 ),
      public_key_ptr( NULL ),
      public_key_length_value( 0 )
{
#ifdef SCSIM_SUPPORT
    if (sim && dynamic_cast<Sip_Sim_Smart_CardGD*>(*sim)){
        // done - DH is implemented on-card
    }
    else
#endif
    {
        dh = new OakleyDH();
        if( dh == NULL )
        {
            throw Mikey_Exception( "Could not create "
                                   "DH parameters." );
        }
    }
}

Key_Agreement_DH_Base::~Key_Agreement_DH_Base()
{
    if (dh)
        delete dh;
    if( peer_key_ptr != NULL )
    {
        delete [] peer_key_ptr;
        peer_key_ptr = NULL;
    }
    if( public_key_ptr != NULL )
    {
        delete [] public_key_ptr;
        public_key_ptr = NULL;
    }
}


Key_Agreement_DH::Key_Agreement_DH( SRef<Certificate_Chain *> cert, SRef<Certificate_Set *> certificate_set )
    : Key_Agreement(),
      Key_Agreement_DH_Base(NULL),
      Peer_Certificates( cert, certificate_set )
{
}

Key_Agreement_DH::Key_Agreement_DH( SRef<Sip_Sim *> s )
    : Key_Agreement(s),
      Key_Agreement_DH_Base(s),
      Peer_Certificates( s->get_certificate_chain(), s->get_cas() )
{
}

Key_Agreement_DH::~Key_Agreement_DH()
{
}

int32_t Key_Agreement_DH::type()
{
    return KEY_AGREEMENT_TYPE_DH;
}

int Key_Agreement_DH_Base::compute_tgk()
{
    assert( peer_key_ptr );

#ifdef SCSIM_SUPPORT
    if (dynamic_cast<Sip_Sim_Smart_CardGD*>(*sim)){
        Sip_Sim_Smart_CardGD *gd = dynamic_cast<Sip_Sim_Smart_CardGD*>(*sim);
        unsigned long len;
        unsigned char *dhval = new unsigned char[192];	//FIXME: fix API to work with unknown key lengths
        gd->get_dhpublic_value(len, dhval);
        gd->gen_tgk( dhval, len );
        return true;
    }
    else
#endif
    {
        int res = dh->compute_secret( peer_key_ptr, peer_key_length_value, tgk(), tgk_length() );
        return res;
    }
}

int Key_Agreement_DH_Base::set_group( int groupValue )
{
#ifdef SCSIM_SUPPORT
    if (sim && dynamic_cast<Sip_Sim_Smart_CardGD*>(*sim))
    {
        Sip_Sim_Smart_CardGD* gd = dynamic_cast<Sip_Sim_Smart_CardGD*>(*sim);
        assert (groupValue==DH_GROUP_OAKLEY5);

        public_key_ptr = new unsigned char[192];

        unsigned long length;
        gd->get_dhpublic_value(length, public_key_ptr);
    }
    else
#endif
    {
        if( !dh->set_group( groupValue ) )
            return 1;

        uint32_t len = dh->secret_length();

        if( len != tgk_length() || !tgk() )
        {
            set_tgk( NULL, len );
        }

        int32_t length = dh->public_key_length();
        if( length != public_key_length_value )
        {
            if( public_key_ptr )
            {
                delete[] public_key_ptr;
            }
            public_key_length_value = length;
            public_key_ptr = new unsigned char[ length ];
        }
        dh->get_public_key( public_key_ptr, length );
    }

    return 0;
}

int Key_Agreement_DH_Base::group()
{
    if( !public_key_ptr )
        return -1;

    if (sim){
        return DH_GROUP_OAKLEY5;
    }else
        return dh->group();
}

void Key_Agreement_DH_Base::set_peer_key( byte_t * peerKey, int peerKeyLength )
{
    if( this->peer_key_ptr )
        delete[] this->peer_key_ptr;

    this->peer_key_ptr = new unsigned char[ peer_key_length_value ];
    this->peer_key_length_value = peerKeyLength;
    memcpy( this->peer_key_ptr, peerKey, peerKeyLength );
}

int Key_Agreement_DH_Base::peer_key_length()
{
    return peer_key_length_value;
}

byte_t * Key_Agreement_DH_Base::peer_key()
{
    return peer_key_ptr;
}

int Key_Agreement_DH_Base::public_key_length()
{
    return public_key_length_value;
}

byte_t * Key_Agreement_DH_Base::public_key()
{
    return public_key_ptr;
}


Mikey_Message* Key_Agreement_DH::create_message()
{
    return Mikey_Message::create( this );
}
