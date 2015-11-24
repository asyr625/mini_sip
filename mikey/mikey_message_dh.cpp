#include "mikey_message_dh.h"

#include "mikey_exception.h"
#include "mikey_payload.h"
#include "mikey_payload_hdr.h"
#include "mikey_payload_t.h"
#include "mikey_payload_rand.h"
#include "mikey_payload_cert.h"
#include "mikey_payload_dh.h"
#include "mikey_payload_err.h"
#include "sip_sim.h"
#include "rand.h"

#include<map>

using namespace std;

Mikey_Message_DH::Mikey_Message_DH()
{
}

Mikey_Message_DH::Mikey_Message_DH( Key_Agreement_DH * ka )
{
    unsigned int csbId = ka->csb_id();
    if( !csbId )
    {
        if(ka->use_sim)
            Rand::randomize( &csbId, sizeof( csbId ), ka->get_sim());
        else
            Rand::randomize( &csbId, sizeof( csbId ));
        ka->set_csb_id( csbId );
    }

    add_payload( new Mikey_Payload_HDR( HDR_DATA_TYPE_DH_INIT, 1,
                                        HDR_PRF_MIKEY_1, csbId,
                                        ka->ncs(), ka->get_cs_id_map_type(),
                                        ka->cs_id_map() ) );

    add_payload( new Mikey_Payload_T() );

    add_policy_to_payload( ka ); //Is in Mikey_Message.cxx

    Mikey_Payload_RAND * payload;

    if(ka->use_sim)
        add_payload(payload = new Mikey_Payload_RAND(ka->get_sim()));
    else
        add_payload( payload = new Mikey_Payload_RAND() );

    //keep a copy of the random value!
    ka->set_rand( payload->rand_data(),
                  payload->rand_length() );

    /* Include the list of certificates if available */
    add_certificate_payloads( ka->certificate_chain() );

    add_payload( new Mikey_Payload_DH( ka->group(), ka->public_key(), ka->key_validity() ) );

    if (ka->use_sim)
    {
        add_signature_payload(ka->get_sim());
    }
    else
    {
        add_signature_payload( ka->certificate_chain()->get_first() );
    }
}

void Mikey_Message_DH::set_offer( Key_Agreement * kaBase )
{
    Key_Agreement_DH* ka = dynamic_cast<Key_Agreement_DH*>(kaBase);
    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a DH keyagreement" );
    }

    SRef<Mikey_Payload *> i = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    bool error = false;
    SRef<Mikey_Message *> errorMessage = new Mikey_Message();

    if( i.is_null() )
    {
        throw Mikey_Exception_Message_Content( "DH init message had no HDR payload" );
    }

#define hdr ((Mikey_Payload_HDR *)*i)
    if( hdr->data_type() != HDR_DATA_TYPE_DH_INIT )
        throw Mikey_Exception_Message_Content( "Expected DH init message" );

    ka->set_ncs( hdr->ncs() );
    ka->set_csb_id( hdr->csb_id() );


    if( hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_SRTP_ID || hdr->cs_id_map_type() == HDR_CS_ID_MAP_TYPE_IPSEC4_ID)
    {
        ka->set_cs_id_map( hdr->cs_id_map() );
        ka->set_cs_id_map_type( hdr->cs_id_map_type() );
    }
    else
    {
        throw Mikey_Exception_Message_Content( "Unknown type of CS ID map" );
    }
    payloads.remove( i );
#undef hdr


    errorMessage->add_payload(
                new Mikey_Payload_HDR( HDR_DATA_TYPE_ERROR, 0,
                                       HDR_PRF_MIKEY_1, ka->csb_id(),
                                       ka->ncs(), ka->get_cs_id_map_type(),
                                       ka->cs_id_map() ) );

    i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    // FIXME i can be NULL
    if( ((Mikey_Payload_T*)*i)->check_offset( MAX_TIME_OFFSET ) )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_TS ) );
    }

    payloads.remove( i );

    add_policy_to_ka(ka); //Is in Mikey_Message.cxx

    i = extract_payload( MIKEYPAYLOAD_RAND_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    ka->set_rand( ((Mikey_Payload_RAND *)*i)->rand_data(),
                  ((Mikey_Payload_RAND *)*i)->rand_length() );

    payloads.remove( i );

    /* If we haven't gotten the peer's certificate chain
     * (for instance during authentication of the message),
     * try to get it now */

    // Fetch peer certificate chain
    SRef<Certificate_Chain *> peerChain = ka->peer_certificate_chain();
    if( peerChain.is_null() || peerChain->get_first().is_null() )
    {
        peerChain = extract_certificate_chain();

        if( !peerChain.is_null() )
        {
            ka->set_peer_certificate_chain( peerChain );
        }
    }

    //FIXME treat the case of an ID payload
    /*
    {
        i = extract_payload( MIKEYPAYLOAD_ID_PAYLOAD_TYPE );
        if( i != NULL ){
            payloads.remove( i );
        }
    }
    */
    i = extract_payload( MIKEYPAYLOAD_DH_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

#define dh ((Mikey_Payload_DH*)*i)

    // FIXME i can be NULL
    if( ka->group() != dh->group() )
    {
        ka->set_group( dh->group() );
    }

    ka->set_peer_key( dh->dh_key(), dh->dh_key_length() );

    ka->set_key_validity( dh->kv() );

    payloads.remove( i );
#undef dh
}

SRef<Mikey_Message *> Mikey_Message_DH::build_response( Key_Agreement * kaBase )
{
    Key_Agreement_DH* ka = dynamic_cast<Key_Agreement_DH*>(kaBase);
    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a DH keyagreement" );
    }

    // Build the response message
    SRef<Mikey_Message *> result = new Mikey_Message();
    result->add_payload(
                new Mikey_Payload_HDR( HDR_DATA_TYPE_DH_RESP, 0,
                                       HDR_PRF_MIKEY_1, ka->csb_id(),
                                       ka->ncs(), ka->get_cs_id_map_type(),
                                       ka->cs_id_map() ) );

    result->add_payload( new Mikey_Payload_T() );

    add_policy_to_payload( ka ); //Is in Mikey_Message.cxx

    /* Include the list of certificates if available */
    result->add_certificate_payloads( ka->certificate_chain() );

    result->add_payload( new Mikey_Payload_DH(
                             ka->group(),
                             ka->public_key(),
                             ka->key_validity() ) );

    result->add_payload( new Mikey_Payload_DH(
                             ka->group(),
                             ka->peer_key(),
                             ka->key_validity() ) );

    if (ka->use_sim)
    {
        result->add_signature_payload(ka->get_sim());
    }
    else
    {
        result->add_signature_payload( ka->certificate_chain()->get_first() );
    }

    return result;
}

SRef<Mikey_Message *> Mikey_Message_DH::parse_response( Key_Agreement  * kaBase )
{
    Key_Agreement_DH* ka = dynamic_cast<Key_Agreement_DH*>(kaBase);

    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a DH keyagreement" );
    }

    SRef<Mikey_Payload *> i = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    bool error = false;
    bool gotDhi = false;
    SRef<Mikey_Message *> errorMessage = new Mikey_Message();
    SRef<Mikey_Cs_Id_Map *> csIdMap;
    uint8_t nCs;

    if( i.is_null() )
    {
        throw Mikey_Exception_Message_Content( "DH resp message had no HDR payload" );
    }

#define hdr ((Mikey_Payload_HDR *)(*i))
    if( hdr->data_type() != HDR_DATA_TYPE_DH_RESP )
    {
        throw Mikey_Exception_Message_Content( "Expected DH resp message" );
    }

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
    //FIXME look at the other fields!

    errorMessage->add_payload(
                new Mikey_Payload_HDR( HDR_DATA_TYPE_ERROR, 0,
                                       HDR_PRF_MIKEY_1, ka->csb_id(),
                                       nCs, ka->get_cs_id_map_type(),
                                       csIdMap ) );

    payloads.remove( i );
    i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    // FIXME i can be NULL
    if( ((Mikey_Payload_T*)*i)->check_offset( MAX_TIME_OFFSET ) )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_TS ) );
    }

    payloads.remove( i );

    add_policy_to_ka(ka); //Is in Mikey_Message.cxx

    // Fetch peer certificate chain
    SRef<Certificate_Chain *> peerChain = ka->peer_certificate_chain();
    if( peerChain.is_null() || peerChain->get_first().is_null() )
    {
        peerChain = extract_certificate_chain();
        if( !peerChain.is_null() )
        {
            ka->set_peer_certificate_chain( peerChain );
        }
    }

    //FIXME treat the case of an ID payload
    /*{
        i = extract_payload( MIKEYPAYLOAD_ID_PAYLOAD_TYPE );
        if( i != NULL ){
            payloads.remove( i );
        }
    }*/

    i = extract_payload( MIKEYPAYLOAD_DH_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    // FIXME i can be NULL
#define dh ((Mikey_Payload_DH *)*i)
    if( string( (const char *)dh->dh_key(), dh->dh_key_length() ) ==
            string( (const char *)ka->public_key(), ka->public_key_length() ) )
    {
        // This is the DHi payload
        gotDhi = true;
    }
    else{
        // This is the DHr payload
        ka->set_peer_key( dh->dh_key(), dh->dh_key_length() );
    }

    payloads.remove( i );
    i = extract_payload( MIKEYPAYLOAD_DH_PAYLOAD_TYPE );

    if( i.is_null() )
    {
        error = true;
        errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
    }

    if( gotDhi )
    {
        // this one should then be DHr
        ka->set_peer_key( dh->dh_key(), dh->dh_key_length() );
    }
    else
    {
        if( string( (const char *)dh->dh_key(), dh->dh_key_length() ) !=
                string( (const char *)ka->public_key(),
                        ka->public_key_length() ) )
        {
            error = true;
            errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_UNSPEC ) );
        }
    }
#undef dh

    //FIXME handle key validity information
    if( error )
    {
        if (ka->use_sim)
        {
            errorMessage->add_signature_payload(ka->get_sim());
        }
        else
        {
            errorMessage->add_signature_payload( ka->certificate_chain()->get_first() );
        }
        throw Mikey_Exception_Message_Content( errorMessage );
    }
    return NULL;
}


bool Mikey_Message_DH::authenticate( Key_Agreement  * kaBase )
{
    Key_Agreement_DH* ka = dynamic_cast<Key_Agreement_DH*>(kaBase);
    if( !ka )
    {
        throw Mikey_Exception_Message_Content( "Not a DH keyagreement" );
    }

    SRef<Mikey_Payload *> sign = (*last_payload());

    // Fetch peer certificate chain
    SRef<Certificate_Chain *> peerCert = ka->peer_certificate_chain();
    if( peerCert.is_null() || peerCert->get_first().is_null() )
    {
        peerCert = extract_certificate_chain();
        if( peerCert.is_null() )
        {
            ka->set_auth_error( "No certificate was found" );
            return true;
        }

        ka->set_peer_certificate_chain( peerCert );
    }

    if( sign->payload_type() != MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE )
    {
        ka->set_auth_error( "No signature payload found" );
        return true;
    }

#define signPl ((Mikey_Payload_SIGN*)*sign)
    int res;
    res = peerCert->get_first()->verif_sign( raw_message_data(),
                                            raw_message_length() - signPl->sig_length(),
                                            signPl->sig_data(),
                                            signPl->sig_length() );
    if( res > 0 ) return false;
    else return true;
}

bool Mikey_Message_DH::is_initiator_message() const
{
    return type() == MIKEY_TYPE_DH_INIT;
}

bool Mikey_Message_DH::is_responder_message() const
{
    return type() == MIKEY_TYPE_DH_RESP;
}

int32_t Mikey_Message_DH::key_agreement_type() const
{
    return KEY_AGREEMENT_TYPE_DH;
}
