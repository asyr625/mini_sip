#include "mikey_message_rsar.h"

#include "mikey_payload_hdr.h"
#include "mikey_payload_t.h"
#include "mikey_payload_rand.h"
#include "mikey_exception.h"
#include "mikey_payload_cert.h"
#include "mikey_payload_key_data.h"
#include "mikey_payload_err.h"
#include "mikey_payload_id.h"
#include "mikey_payload_kemac.h"
#include "mikey_payload_v.h"
#include "mikey_payload_pke.h"

#include "rand.h"

using namespace std;

Mikey_Message_RSAR::Mikey_Message_RSAR()
{
}

Mikey_Message_RSAR::Mikey_Message_RSAR( Key_Agreement_RSAR* ka )
{
    unsigned int csbId = ka->csb_id();

    if( !csbId )
    {
        Rand::randomize( &csbId, sizeof( csbId ));
        ka->set_csb_id( csbId );
    }

    Mikey_Payload_T* tPayload;
    Mikey_Payload_RAND* randPayload;

    //adding header payload
    add_payload(new Mikey_Payload_HDR(HDR_DATA_TYPE_RSA_R_INIT, 1,
                                      HDR_PRF_MIKEY_1, csbId, ka->ncs(),
                                      ka->get_cs_id_map_type(), ka->cs_id_map()));

    //adding timestamp payload
    add_payload(tPayload = new Mikey_Payload_T());

    //adding security policy
    add_policy_to_payload(ka); //Is in Mikey_Message.cxx

    //keep a copy of the time stamp
    uint64_t t = tPayload->ts();
    ka->set_tsent(t);

    //adding random payload
    add_payload(randPayload = new Mikey_Payload_RAND());

    //keep a copy of the random value
    ka->set_rand(randPayload->rand_data(), randPayload->rand_length());

    // Add initiator identity (IDi)
    add_id( ka->uri() );

    // Add certificate chain (SIGN)
    add_certificate_payloads( ka->certificate_chain() );

    // Add responder identity (IDr)
    if( !ka->peer_uri().empty() )
    {
        add_id( ka->peer_uri() );
    }

    // Add signature (T)
    add_signature_payload( ka->certificate_chain()->get_first() );
}

void Mikey_Message_RSAR::set_offer( Key_Agreement * kaBase )
{
    Key_Agreement_RSAR* ka = dynamic_cast<Key_Agreement_RSAR*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a RSAR keyagreement" );
    }

    SRef<Mikey_Payload*> i = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    bool error = false;
    //uint32_t csbId;
    SRef<Mikey_Cs_Id_Map*> csIdMap;
    SRef<Mikey_Message*> errorMessage = new Mikey_Message();
    //uint8_t nCs;

    if( i.is_null() || i->payload_type() != MIKEYPAYLOAD_HDR_PAYLOAD_TYPE )
    {
        throw Mikey_Exception_Message_Content( "RSAR init message had no HDR payload" );
    }

#define hdr ((Mikey_Payload_HDR *)(*i))
    if( hdr->data_type() != HDR_DATA_TYPE_RSA_R_INIT )
    {
        throw Mikey_Exception_Message_Content( "Expected RSAR init message" );
    }

    ka->set_ncs( hdr->ncs() );
    ka->set_csb_id( hdr->csb_id() );
    ka->set_v(hdr->v());

    if( hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_SRTP_ID || hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_IPSEC4_ID )
    {
        ka->set_cs_id_map( hdr->cs_id_map() );
        ka->set_cs_id_map_type( hdr->cs_id_map_type() );
    }
    else
    {
        throw Mikey_Exception_Message_Content( "Unknown type of CS ID map" );
    }


#undef hdr
    errorMessage->add_payload(
                new Mikey_Payload_HDR( HDR_DATA_TYPE_ERROR, 0,
                                       HDR_PRF_MIKEY_1, ka->csb_id(),
                                       ka->ncs(), ka->get_cs_id_map_type(),
                                       ka->cs_id_map() ) );

    //FIXME look at the other fields!
    remove( i );
    i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );

    if( i.is_null() )
        throw Mikey_Exception_Message_Content( "RSAR init message had no T payload" );

    // FIXME i can be NULL
#define plT ((Mikey_Payload_T *)(*i))
    if( plT->check_offset( MAX_TIME_OFFSET ) )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_TS ) );
    }
    ka->t_received = plT->ts();

    remove( i );
#undef plT

    add_policy_to_ka(ka); //Is in Mikey_Message.cxx

    i = extract_payload( MIKEYPAYLOAD_RAND_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

#define plRand ((Mikey_Payload_RAND *)*i)
    // FIXME i can be NULL
    ka->set_rand( plRand->rand_data(), plRand->rand_length() );

    remove( i );
#undef plRand
    i = extract_payload( MIKEYPAYLOAD_ID_PAYLOAD_TYPE );

    //FIXME treat the case of an ID payload
    if( !i.is_null() )
    {
        remove( i );
    }
}

SRef<Mikey_Message *> Mikey_Message_RSAR::build_response( Key_Agreement * kaBase )
{
    Key_Agreement_RSAR* ka = dynamic_cast<Key_Agreement_RSAR*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a RSAR keyagreement" );
    }

    // Build the response message
    SRef<Mikey_Message_RSAR *> result = new Mikey_Message_RSAR();
    result->add_payload(
                new Mikey_Payload_HDR( HDR_DATA_TYPE_RSA_R_RESP, 0,
                                       HDR_PRF_MIKEY_1, ka->csb_id(),
                                       ka->ncs(), ka->get_cs_id_map_type(),
                                       ka->cs_id_map() ) );

    Mikey_Payload_T* tPayload = new Mikey_Payload_T();
    result->add_payload( tPayload );

    //keep a copy of the time stamp
    uint64_t t = tPayload->ts();
    ka->set_tsent(t);

    //adding random payload
    Mikey_Payload_RAND* randPayload = NULL;
    result->add_payload(randPayload = new Mikey_Payload_RAND());

    // Add IDr
    result->add_id( ka->uri() );

    // Add certificate chain
    result->add_certificate_payloads( ka->certificate_chain() );

    // TODO move encrAlg and macAlg to method or ctor parameter
    int encrAlg = MIKEY_ENCR_AES_CM_128;
    int macAlg = MIKEY_MAC_HMAC_SHA1_160;

    result->add_pke_kemac( ka, encrAlg, macAlg );

    result->add_signature_payload( ka->certificate_chain()->get_first(),
                                   // 				     false );
                                   true );

    return *result;
}

SRef<Mikey_Message *> Mikey_Message_RSAR::parse_response( Key_Agreement  * kaBase )
{
    Key_Agreement_RSAR* ka = dynamic_cast<Key_Agreement_RSAR*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a RSAR keyagreement" );
    }

    SRef<Mikey_Payload *> i = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    bool error = false;
    SRef<Mikey_Message *> errorMessage = new Mikey_Message();
    SRef<Mikey_Cs_Id_Map *> csIdMap;
    uint8_t nCs;

    if( i.is_null() || i->payload_type() != MIKEYPAYLOAD_HDR_PAYLOAD_TYPE )
    {
        throw Mikey_Exception_Message_Content( "RSAR response message had no HDR payload" );
    }

#define hdr ((Mikey_Payload_HDR *)(*i))
    if( hdr->data_type() != HDR_DATA_TYPE_RSA_R_RESP )
        throw Mikey_Exception_Message_Content( "Expected RSAR response message" );

    if( hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_SRTP_ID || hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_IPSEC4_ID)
    {
        csIdMap = hdr->cs_id_map();
    }
    else
    {
        throw Mikey_Exception_Message_Content( "Unknown type of CS ID map" );
    }

    nCs = hdr->ncs();
#undef hdr
    ka->set_cs_id_map( csIdMap );

    errorMessage->add_payload(
                new Mikey_Payload_HDR( HDR_DATA_TYPE_ERROR, 0,
                                       HDR_PRF_MIKEY_1, ka->csb_id(),
                                       nCs, HDR_CS_ID_MAP_TYPE_SRTP_ID,
                                       csIdMap ) );


    remove( i );
    i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    // FIXME i can be NULL
#define plT ((Mikey_Payload_T*)*i)
    if( plT->check_offset( MAX_TIME_OFFSET ) )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_TS ) );
    }

    uint64_t t_received = plT->ts();
#undef plT

    i = extract_payload( MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    // FIXME handle i == NULL
#define kemac ((Mikey_Payload_KEMAC *)*i)
    int encrAlg = kemac->encr_alg();
    int macAlg  = kemac->mac_alg();
    ka->mac_alg = macAlg;

    // Derive the transport keys
    byte_t * encrKey = NULL;
    byte_t * iv = NULL;
    unsigned int encrKeyLength = 0;

    if( !derive_transp_keys( ka, encrKey, iv, encrKeyLength,
                             encrAlg, macAlg, t_received,
                             *errorMessage ) )
    {
        if( encrKey != NULL )
            delete [] encrKey;
        if( iv != NULL )
            delete [] iv;

        unsigned int authKeyLength = 20;
        byte_t* authKey = new byte_t[ authKeyLength ];
        ka->gen_transp_auth_key( authKey, authKeyLength );

        errorMessage->add_vpayload( MIKEY_MAC_HMAC_SHA1_160,
                                    ka->t_received, authKey, authKeyLength  );

        delete [] authKey;
        throw Mikey_Exception_Message_Content( errorMessage );
    }

    // decrypt the TGK
    SRef<Mikey_Payloads*> subPayloads =
            kemac->decode_payloads( MIKEYPAYLOAD_ID_PAYLOAD_TYPE,
                                    encrKey, encrKeyLength, iv );

    if( encrKey != NULL )
    {
        delete [] encrKey;
        encrKey = NULL;
    }
    if( iv != NULL )
    {
        delete [] iv;
        iv = NULL;
    }

    string peerUri = subPayloads->extract_id_str( 0 );
    if( peerUri.empty() || peerUri != ka->peer_uri()  )
    {
        cerr << "Encrypted IDr mismatch" << endl;

        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_ID ) );
    }

    SRef<Mikey_Payload *> plKeyData =
            subPayloads->extract_payload( MIKEYPAYLOAD_KEYDATA_PAYLOAD_TYPE );
    if( plKeyData )
    {
        Mikey_Payload_Key_Data *keyData =
                dynamic_cast<Mikey_Payload_Key_Data*>(*plKeyData);

        int tgkLength = keyData->key_data_length();
        byte_t * tgk = keyData->key_data();

        ka->set_tgk( tgk, tgkLength );
        ka->set_key_validity( keyData->kv() );
    }
    else
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }
#undef kemac

    if( error )
    {
        byte_t authKey[20];
        unsigned int authKeyLength = 20;

        ka->gen_transp_auth_key( authKey, 20 );

        errorMessage->add_vpayload( MIKEY_MAC_HMAC_SHA1_160,
                                    t_received, authKey, authKeyLength  );

        throw Mikey_Exception_Message_Content( errorMessage );
    }
    add_policy_to_ka(ka); //Is in Mikey_Message.cxx

    return NULL;
}


bool Mikey_Message_RSAR::authenticate( Key_Agreement  * kaBase )
{
    Key_Agreement_RSAR* ka = dynamic_cast<Key_Agreement_RSAR*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a RSAR keyagreement" );
    }

    SRef<Mikey_Payload *> payload = *(last_payload());
    list<Mikey_Payload *>::iterator payload_i;

    if( ka->rand() == NULL )
    {
        SRef<Mikey_Payload *> pl = extract_payload(MIKEYPAYLOAD_RAND_PAYLOAD_TYPE );

        if( pl.is_null() )
        {
            ka->set_auth_error( "The MIKEY init has no RAND payload." );
            return true;
        }

        Mikey_Payload_RAND * randPayload;

        randPayload = (Mikey_Payload_RAND*)*pl;

        ka->set_rand( randPayload->rand_data(), randPayload->rand_length() );
    }

    if( is_initiator_message() || is_responder_message() )
    {
        if( payload->payload_type() != MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE)
        {
            throw Mikey_Exception( "RSAR init did not end with a SIGN payload" );
        }

        if( is_responder_message() && ka->csb_id() != csb_id() )
        {
            ka->set_auth_error( "CSBID mismatch\n" );
            return true;
        }

        // Check Peer ID (IDi resp IDr)
        string peerUri = extract_id_str( 0 );
        if( !peerUri.empty() )
        {
            if( !ka->peer_uri().empty() )
            {
                if( peerUri != ka->peer_uri() )
                {
                    cerr << "Peer ID mismatch " + peerUri + " != " + ka->peer_uri() << endl;
                    ka->set_auth_error( "Peer ID mismatch" );
                    return true;
                }
#ifdef DEBUG_OUTPUT
                cerr << "Peer ID authenticated " << peerUri << endl;
#endif
            }
            else{
                ka->set_peer_uri( peerUri );
            }
        }

        // Check My ID (IDr)
        if( is_initiator_message() )
        {
            string uri = extract_id_str( 1 );
            if( !uri.empty() )
            {
                if( uri != ka->uri() )
                {
                    cerr << "ID mismatch" << endl;
                    ka->set_auth_error( "ID mismatch" );
                    return true;
                }
#ifdef DEBUG_OUTPUT
                cerr << "ID match" << endl;
#endif
            }
        }

        // Fetch peer certificate chain
        SRef<Certificate_Chain *> peerChain = ka->peer_certificate_chain();
        if( peerChain.is_null() || peerChain->get_first().is_null() )
        {
            peerChain = extract_certificate_chain();
            if( peerChain.is_null() )
            {
                ka->set_auth_error( "No certificate was found" );
                return true;
            }

            ka->set_peer_certificate_chain( peerChain );
        }

        if( !verify_signature( peerChain->get_first(), is_responder_message() ) )
        {
            cout << "Verification of the RSAR init message SIGN payload failed!"  << endl;
            cout << "Keypair of the initiator probably mismatch!" << endl;
            return true;
        }

        ka->set_csb_id( csb_id() );

        if( is_responder_message() )
        {
            if( !extract_pke_env_key( ka ) )
            {
                throw Mikey_Exception( "Decryption of envelope key failed" );
            }

            if( !verify_kemac( ka, true ) )
                return true;
        }
        return false;
    }
    else
    {
        throw Mikey_Exception( "Invalide type for a RSAR message" );
    }
}

bool Mikey_Message_RSAR::is_initiator_message() const
{
    return type() == MIKEY_TYPE_RSA_R_INIT;
}

bool Mikey_Message_RSAR::is_responder_message() const
{
    return type() == MIKEY_TYPE_RSA_R_RESP;
}

int32_t Mikey_Message_RSAR::key_agreement_type() const
{
    return KEY_AGREEMENT_TYPE_RSA_R;
}
