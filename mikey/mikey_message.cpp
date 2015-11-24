#include "mikey_message.h"

#include "mikey_payload.h"
#include "mikey_payload_hdr.h"
#include "mikey_payload_kemac.h"
#include "mikey_payload_pke.h"
#include "mikey_payload_dh.h"
#include "mikey_payload_sign.h"
#include "mikey_payload_t.h"
#include "mikey_payload_id.h"
#include "mikey_payload_cert.h"
#include "mikey_payload_chash.h"
#include "mikey_payload_v.h"
#include "mikey_payload_sp.h"
#include "mikey_payload_rand.h"
#include "mikey_payload_err.h"
#include "mikey_payload_key_data.h"
#include "mikey_payload_general_extensions.h"
#include "mikey_exception.h"

#include "aes.h"
#include "base64.h"
#include "hmac.h"
#include "cert.h"
#include "sip_sim.h"

#include<map>
#include<string.h>

#include "mikey_message_dh.h"
#include "mikey_message_psk.h"
#include "mikey_message_pke.h"
#include "mikey_message_dhhmac.h"
#include "mikey_message_rsar.h"


// The signature calculation will be factor two faster if this
// guess is correct (128 bytes == 1024 bits)
#define GUESSED_SIGNATURE_LENGTH 128

using namespace std;

Mikey_Payloads::Mikey_Payloads()
    : compiled(false), rawData(NULL)
{
}

Mikey_Payloads::Mikey_Payloads( int firstPayloadType, byte_t *message, int lengthLimit )
    : compiled(true), rawData(message)
{
    parse( firstPayloadType, message, lengthLimit, payloads );
}

Mikey_Message::Mikey_Message()
{
}

Mikey_Message* Mikey_Message::create( Key_Agreement_DH * ka )
{
    return new Mikey_Message_DH( ka );
}

Mikey_Message* Mikey_Message::create( Key_Agreement_DHHMAC * ka, int macAlg )
{
    return new Mikey_Message_DHHMAC( ka, macAlg );
}

Mikey_Message* Mikey_Message::create( Key_Agreement_PSK * ka, int encrAlg, int macAlg )
{
    return new Mikey_Message_PSK( ka, encrAlg, macAlg );
}

Mikey_Message* Mikey_Message::create(Key_Agreement_PKE* ka, int encrAlg, int macAlg)
{
    return new Mikey_Message_PKE( ka, encrAlg, macAlg );
}

Mikey_Message* Mikey_Message::create(Key_Agreement_RSAR* ka)
{
    return new Mikey_Message_RSAR( ka );
}

Mikey_Message* Mikey_Message::parse( byte_t *message, int lengthLimit )
{
    std::list<SRef<Mikey_Payload*> > payloads;

    Mikey_Payloads::parse( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE,
                           message, lengthLimit, payloads );

    if( payloads.size() == 0 )
    {
        throw Mikey_Exception_Message_Content( "No payloads" );
    }

    Mikey_Payload_HDR *hdr =
            dynamic_cast<Mikey_Payload_HDR*>(**payloads.begin());

    if( !hdr )
    {
        throw Mikey_Exception_Message_Content( "No header in the payload" );
    }

    Mikey_Message* msg = NULL;

    switch( hdr->data_type() ){
    case MIKEY_TYPE_DH_INIT:
    case MIKEY_TYPE_DH_RESP:
        msg = new Mikey_Message_DH();
        break;
    case MIKEY_TYPE_PSK_INIT:
    case MIKEY_TYPE_PSK_RESP:
        msg = new Mikey_Message_PSK();
        break;
    case MIKEY_TYPE_PK_INIT:
    case MIKEY_TYPE_PK_RESP:
        msg = new Mikey_Message_PKE();
        break;
    case MIKEY_TYPE_DHHMAC_INIT:
    case MIKEY_TYPE_DHHMAC_RESP:
        msg = new Mikey_Message_DHHMAC();
        break;
    case MIKEY_TYPE_RSA_R_INIT:
    case MIKEY_TYPE_RSA_R_RESP:
        msg = new Mikey_Message_RSAR();
        break;
    case MIKEY_TYPE_ERROR:
        msg = new Mikey_Message();
        break;
    default:
        throw Mikey_Exception_Unimplemented( "Unimplemented type of message in INVITE" );
    }

    msg->set_raw_message_data( message );
    msg->payloads = payloads;

    return msg;
}

Mikey_Message* Mikey_Message::parse( std::string b64Message )
{
    int messageLength;
    byte_t * messageData;

    messageData = base64_decode( b64Message, &messageLength );

    if( messageData == NULL )
    {
        throw Mikey_Exception_Message_Content( "Invalid B64 input message" );
    }
    return parse( messageData, messageLength );
}

Mikey_Message::~Mikey_Message()
{
}

Mikey_Payloads::~Mikey_Payloads()
{
    if( rawData )
        delete [] rawData;
    rawData = NULL;
}

static SRef<Mikey_Payload*> parse_payload( int payloadType, byte_t * msgpos, int limit )
{
    SRef<Mikey_Payload*> payload = NULL;

    switch (payloadType)
    {
    case MIKEYPAYLOAD_HDR_PAYLOAD_TYPE:
        payload = new Mikey_Payload_HDR(msgpos, limit);
        break;
    case MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE:
        payload = new Mikey_Payload_KEMAC(msgpos, limit);
        break;
    case MIKEYPAYLOAD_PKE_PAYLOAD_TYPE:
        payload = new Mikey_Payload_PKE(msgpos, limit);
        break;
    case MIKEYPAYLOAD_DH_PAYLOAD_TYPE:
        payload = new Mikey_Payload_DH(msgpos, limit);
        break;
    case MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE:
        payload = new Mikey_Payload_SIGN(msgpos, limit);
        break;
    case MIKEYPAYLOAD_T_PAYLOAD_TYPE:
        payload = new Mikey_Payload_T(msgpos, limit);
        break;
    case MIKEYPAYLOAD_ID_PAYLOAD_TYPE:
        payload = new Mikey_Payload_ID(msgpos, limit);
        break;
    case MIKEYPAYLOAD_CERT_PAYLOAD_TYPE:
        payload = new Mikey_Payload_CERT(msgpos, limit);
        break;
    case MIKEYPAYLOAD_CHASH_PAYLOAD_TYPE:
        payload = new Mikey_Payload_CHASH(msgpos, limit);
        break;
    case MIKEYPAYLOAD_V_PAYLOAD_TYPE:
        payload = new Mikey_Payload_V(msgpos, limit);
        break;
    case MIKEYPAYLOAD_SP_PAYLOAD_TYPE:
        payload = new Mikey_Payload_SP(msgpos, limit);
        break;
    case MIKEYPAYLOAD_RAND_PAYLOAD_TYPE:
        payload = new Mikey_Payload_RAND(msgpos, limit);
        break;
    case MIKEYPAYLOAD_ERR_PAYLOAD_TYPE:
        payload = new Mikey_Payload_ERR(msgpos, limit);
        break;
    case MIKEYPAYLOAD_KEYDATA_PAYLOAD_TYPE:
        payload = new Mikey_Payload_Key_Data(msgpos, limit);
        break;
    case MIKEYPAYLOAD_GENERALEXTENSIONS_PAYLOAD_TYPE:
        payload = new Mikey_Payload_General_Extensions(msgpos, limit);
        break;

    case MIKEYPAYLOAD_LAST_PAYLOAD:
        break;
    default:
        throw Mikey_Exception_Message_Content( "Payload of unrecognized type." );
    }

    return payload;
}

void Mikey_Payloads::parse( int firstPayloadType, byte_t *message, int lengthLimit, std::list<SRef<Mikey_Payload*> >& payloads)
{
    SRef<Mikey_Payload*> hdr;
    byte_t * msgpos = message;
    int limit = lengthLimit;

    hdr = parse_payload( firstPayloadType, message, limit );

    payloads.push_back( hdr );

    limit -=  (int)( hdr->end() - msgpos );
    msgpos = hdr->end();

    int nextPayloadType = hdr->next_payload_type();

    while( !(msgpos >= message + lengthLimit ) && nextPayloadType != Mikey_Payload::Last_Payload)
    {

        SRef<Mikey_Payload*> payload = parse_payload( nextPayloadType, msgpos, limit );

        nextPayloadType = payload->next_payload_type();
        payloads.push_back( payload );

        assert(( payload->end() - msgpos ) == ( payload->length() ));
        limit -= (int)( payload->end() - msgpos );
        msgpos = payload->end();
    }

    if(! (msgpos == message + lengthLimit && nextPayloadType==MIKEYPAYLOAD_LAST_PAYLOAD ) )
        throw Mikey_Exception_Message_Length_Exception(
                "The length of the message did not match"
                "the total length of payloads." );
}


void Mikey_Payloads::add_payload( SRef<Mikey_Payload*> payload )
{
    compiled = false;
    // Put the nextPayloadType in the previous payload */
    if( payload->payload_type() != MIKEYPAYLOAD_HDR_PAYLOAD_TYPE )
    {
        list<SRef<Mikey_Payload*> >::reverse_iterator i = payloads.rbegin();
        if( i != payloads.rend() )
        {
            (*i)->set_next_payload_type( payload->payload_type() );
        }
    }

    payloads.push_back( payload );
}

void Mikey_Payloads::operator+=( SRef<Mikey_Payload*> payload )
{
    add_payload(payload);
}


static vector<byte_t> tsToVec( uint64_t ts )
{
    vector<byte_t> vec;

    vec.resize( 8 );
    for( int i = 0; i < 8; i++ )
    {
        vec[ 8 - i - 1 ] =
                (byte_t)((ts >> (i*8))&0xFF);
    }
    return vec;
}

std::vector<byte_t> Mikey_Payloads::build_sign_data( size_t sigLength, bool useIdsT )
{
    vector<byte_t> signData;

    byte_t * start = raw_message_data();
    byte_t * end = start;
    int diff = raw_message_length() - (int)sigLength;
    assert(diff>=0);
    end += diff;

    signData.insert( signData.end(), start, end);

    if( useIdsT )
    {
        vector<byte_t> vecIDi = extract_id_vec( 0 );
        vector<byte_t> vecIDr = extract_id_vec( 1 );
        SRef<Mikey_Payload*> i;

        i = extract_payload( MIKEYPAYLOAD_T_PAYLOAD_TYPE );
        if( !i ){
            throw Mikey_Exception( "Could not perform digital signature of the message, no T" );
        }

        SRef<Mikey_Payload_T*> plT = dynamic_cast<Mikey_Payload_T*>(*i);
        vector<byte_t> vecTs = tsToVec( plT->ts() );

        signData.insert( signData.end(), vecIDi.begin(), vecIDi.end() );
        signData.insert( signData.end(), vecIDr.begin(), vecIDr.end() );
        signData.insert( signData.end(), vecTs.begin(), vecTs.end() );
    }

    return signData;
}


void Mikey_Payloads::add_signature_payload( SRef<Sip_Sim*> sim, bool addIdsAndT )
{
    byte_t signature[4096];
    int signatureLength=4096;
    Mikey_Payload_SIGN * sign;
    SRef<Mikey_Payload*> last;
    vector<byte_t> signData;

    // set the previous nextPayloadType to signature
    last = *last_payload();
    last->set_next_payload_type( MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE );

    // See the comment in addSignaturePayload(cert) for explanation of
    // the following steps.

    add_payload( ( sign = new Mikey_Payload_SIGN( GUESSED_SIGNATURE_LENGTH,
                                                  MIKEYPAYLOAD_SIGN_TYPE_RSA_PKCS ) ) );

    signData = build_sign_data( GUESSED_SIGNATURE_LENGTH, addIdsAndT );

    if (!sim->get_signature( &signData.front(), (int)signData.size(),
                            signature, signatureLength, true ))
    {
        throw Mikey_Exception( "Could not perform digital signature of the message" );
    }

    if (signatureLength!=GUESSED_SIGNATURE_LENGTH){	// if the length field in the signature payload was
        // wrong, we have to redo the signature
        sign->set_sig_data(signature, signatureLength); // the length needs to be set to the correct value
        signData = build_sign_data( signatureLength, addIdsAndT );

        sim->get_signature( &signData.front(), (int)signData.size(),
                           signature, signatureLength, true );
    }

    sign->set_sig_data( signature, signatureLength );
    compiled = false;
}

void Mikey_Payloads::add_signature_payload( SRef<Certificate *> cert, bool addIdsAndT )
{
    byte_t signature[4096];
    int signatureLength = 128;
    Mikey_Payload_SIGN * sign;
    SRef<Mikey_Payload*> last;
    vector<byte_t> signData;

    // set the previous nextPayloadType to signature
    last = *last_payload();
    last->set_next_payload_type( MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE );

    //The SIGN payload constructor can not take the signature as
    //parameter. This is because it can not be computed before
    //the SIGN payload has been added to the MIKEY message (the next
    //payload field in the payload before SIGN is not set).
    //
    //We guess that the length of the signature is 1024 bits. We then
    //calculate the signature, and if it turns out that it was not
    //1024, we have to re-do the signature calculation with the correct
    //length.
    //
    //Double-signatures would be avoided if the Certificate had a
    //method to find out the length of the signature.

    add_payload( ( sign = new Mikey_Payload_SIGN(GUESSED_SIGNATURE_LENGTH, MIKEYPAYLOAD_SIGN_TYPE_RSA_PKCS ) ) );

    signData = build_sign_data( GUESSED_SIGNATURE_LENGTH, addIdsAndT );

    if (cert->sign_data( &signData.front(), (int)signData.size(), signature, &signatureLength ))
    {
        throw Mikey_Exception( "Could not perform digital signature of the message" );
    }


    if (signatureLength!=GUESSED_SIGNATURE_LENGTH){	// if the length field in the signature payload was
        // wrong, we have to redo the signature
        sign->set_sig_data(signature, signatureLength); // the length needs to be set to the correct value
        signData = build_sign_data( signatureLength, addIdsAndT );

        cert->sign_data( &signData.front(), (int)signData.size(),
                        signature, &signatureLength );
    }

    sign->set_sig_data( signature, signatureLength ); // the payload signature is a dummy value until we do this
    compiled = false;
}

void Mikey_Payloads::add_kemac_payload( byte_t * tgk, int tgkLength, byte_t * encrKey, byte_t * iv, byte_t * authKey,
                                        int encrAlg, int macAlg, bool kemacOnly )
{
    byte_t * encrData = new byte_t[ tgkLength ];
    AES * aes;
    SRef<Mikey_Payload*> last;

    // set the previous nextPayloadType to KEMAC
    last = * last_payload();
    last->set_next_payload_type( MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE );

    switch( encrAlg )
    {
    case MIKEY_PAYLOAD_KEMAC_ENCR_AES_CM_128:
        aes = new AES( encrKey, 16 );
        aes->ctr_encrypt( tgk, tgkLength, encrData, iv );
        delete aes;
        break;
    case MIKEY_PAYLOAD_KEMAC_ENCR_NULL:
        memcpy( encrData, tgk, tgkLength );
        break;
    case MIKEY_PAYLOAD_KEMAC_ENCR_AES_KW_128:
        //TODO
    default:
        delete [] encrData;
        throw Mikey_Exception( "Unknown encryption algorithm" );
    }

    Mikey_Payload_KEMAC * payload;
    byte_t macData[20];
    unsigned int macDataLength;
    byte_t* macInput = NULL;
    byte_t* macInputPtr = NULL;
    unsigned int macInputLength = 0;

    payload = new Mikey_Payload_KEMAC( encrAlg, tgkLength, encrData, macAlg, macData );
    add_payload( payload );

    if( kemacOnly ){
        macInputLength = payload->length();
        macInput = new byte_t[ macInputLength ];
        payload->write_data( macInput, macInputLength );
        macInput[0] = MIKEYPAYLOAD_LAST_PAYLOAD;
        macInputPtr = macInput;
    }
    else{
        macInputPtr = raw_message_data();
        macInputLength = raw_message_length();
    }

    switch( macAlg )
    {
    case MIKEY_PAYLOAD_KEMAC_MAC_HMAC_SHA1_160:
    {
        hmac_sha1( authKey, 20,
                   macInputPtr,
                   // Compute the MAC over the mac input,
                   // MAC field excluded
                   macInputLength - 20,
                   macData, &macDataLength );

        //assert( macDataLength == 20 );
        payload->set_mac( macData );
        break;
    }
    case MIKEY_PAYLOAD_KEMAC_MAC_NULL:
        break;
    default:
        delete [] encrData;
        throw Mikey_Exception( "Unknown MAC algorithm" );
    }
    compiled = false;
    delete [] encrData;
    if( macInput )
    {
        delete[] macInput;
        macInput = NULL;
    }
}


void Mikey_Payloads::add_vpayload( int macAlg, uint64_t t, byte_t * authKey, uint32_t authKeyLength)
{
    Mikey_Payload_V * payload;
    unsigned int hmacOutputLength;
    byte_t hmacOutput[20];
    byte_t * hmacInput;
    unsigned int messageLength;
    byte_t * messageData;

    SRef<Mikey_Payload*> last;
    // set the previous nextPayloadType to V
    last = *last_payload();
    last->set_next_payload_type( MIKEYPAYLOAD_V_PAYLOAD_TYPE );

    switch( macAlg )
    {
    case MIKEY_PAYLOAD_V_MAC_HMAC_SHA1_160:
        add_payload( payload = new Mikey_Payload_V( macAlg, hmacOutput ) );

        messageLength = raw_message_length();
        messageData = raw_message_data();

        //hmacInput = [ message t_received ]
        hmacInput = new byte_t[ messageLength + 8 -20 ];

        memcpy( hmacInput, messageData, messageLength-20 );

        hmacInput[ messageLength - 20 ] = (byte_t)((t >> 56)&0xFF);
        hmacInput[ messageLength - 19 ] = (byte_t)((t >> 48)&0xFF);
        hmacInput[ messageLength - 18 ] = (byte_t)((t >> 40)&0xFF);
        hmacInput[ messageLength - 17 ] = (byte_t)((t >> 32)&0xFF);
        hmacInput[ messageLength - 16 ] = (byte_t)((t >> 24)&0xFF);
        hmacInput[ messageLength - 15 ] = (byte_t)((t >> 16)&0xFF);
        hmacInput[ messageLength - 14 ] = (byte_t)((t >>  8)&0xFF);
        hmacInput[ messageLength - 13 ] = (byte_t)((t      )&0xFF);

        hmac_sha1( authKey, authKeyLength,
                   hmacInput, messageLength + 8 -20,
                   hmacOutput, &hmacOutputLength );

        payload->set_mac( hmacOutput );
        delete [] hmacInput;
        break;

    case MIKEY_PAYLOAD_V_MAC_NULL:
        add_payload( new Mikey_Payload_V( macAlg, NULL ) );
        break;
    default:
        throw Mikey_Exception( "Unknown MAC algorithm" );
    }
    compiled = false;
}


void Mikey_Payloads::compile()
{
    if (compiled)
        throw Mikey_Exception_Message_Content("BUG: trying to compile already compiled message.");

    if( rawData )
        delete [] rawData;

    rawData = new byte_t[ raw_message_length() ];

    list<SRef<Mikey_Payload*> >::iterator i;
    byte_t *pos = rawData;
    for (i=payloads.begin(); i!=payloads.end(); i++)
    {
        int len = (*i)->length();
        (*i)->write_data(pos,len);
        pos += len;
    }
}


std::string Mikey_Payloads::debug_dump()
{
    string ret = "";
    list<SRef<Mikey_Payload*> >::iterator i;
    for (i = payloads.begin(); i != payloads.end(); i++)
    {
        ret = ret + "\n\n" + (*i)->debug_dump();
    }
    return ret;
}

byte_t * Mikey_Payloads::raw_message_data()
{
    if (!compiled)
        compile();
    return rawData;
}

int Mikey_Payloads::raw_message_length()
{
    list<SRef<Mikey_Payload*> >::iterator i;
    int length = 0;
    for (i = payloads.begin(); i != payloads.end(); i++)
    {
        length += (*i)->length();
    }

    return length;
}

void Mikey_Payloads::set_raw_message_data( byte_t *data )
{
    if( rawData )
    {
        delete[] rawData;
        rawData = NULL;
    }
    rawData = data;
    compiled = true;
}


std::list<SRef<Mikey_Payload*> >::const_iterator Mikey_Payloads::first_payload() const
{
    return payloads.begin();
}
std::list<SRef<Mikey_Payload*> >::const_iterator Mikey_Payloads::last_payload() const
{
    return --payloads.end();
}

std::list<SRef<Mikey_Payload*> >::iterator Mikey_Payloads::first_payload()
{
    return payloads.begin();
}

std::list<SRef<Mikey_Payload*> >::iterator Mikey_Payloads::last_payload()
{
    return --payloads.end();
}

std::string Mikey_Payloads::b64_message()
{
    return base64_encode( raw_message_data(), raw_message_length() );
}

int Mikey_Message::type() const
{
    SRef<const Mikey_Payload*> hdr = extract_payload( MIKEYPAYLOAD_HDR_PAYLOAD_TYPE );
    if( hdr.is_null() )
    {
        throw Mikey_Exception_Message_Content( "No header in the payload" );
    }
    return dynamic_cast<const Mikey_Payload_HDR *>(*hdr)->data_type();
}

uint32_t Mikey_Message::csb_id()
{
    SRef<Mikey_Payload*> hdr = * first_payload();
    if( hdr->payload_type() != MIKEYPAYLOAD_HDR_PAYLOAD_TYPE )
    {
        throw Mikey_Exception_Message_Content( "First payload was not a header" );
    }
    return dynamic_cast<Mikey_Payload_HDR *>(*hdr)->csb_id();
}

SRef<Mikey_Payload*> Mikey_Payloads::extract_payload( int payloadType )
{
    list<SRef<Mikey_Payload*> >::iterator i;

    for( i = payloads.begin(); i != payloads.end(); i++ )
    {
        if( (*i)->payload_type() == payloadType )
        {
            return *i;
        }
    }
    return NULL;
}

SRef<const Mikey_Payload*> Mikey_Payloads::extract_payload( int payloadType ) const
{
    list<SRef<Mikey_Payload*> >::const_iterator i;

    for( i = payloads.begin(); i != payloads.end(); i++ )
    {
        if( (*i)->payload_type() == payloadType )
        {
            return **i;
        }
    }
    return NULL;
}

void Mikey_Payloads::remove( SRef<Mikey_Payload*> payload)
{
    list<SRef<Mikey_Payload*> >::iterator i;

    for( i = payloads.begin(); i != payloads.end(); i++ )
    {
        if( *i == payload )
        {
            payloads.erase( i );
            return;
        }
    }
}

void Mikey_Payloads::add_policy_to_payload(Key_Agreement * ka)
{
#define comp (uint16_t)((*iter)->policy_no) << 8 | (uint16_t)((*iter)->prot_type)
    // Adding policy to payload
    Mikey_Payload_SP *PSP;
    list <Policy_Type *> * policy = ka->get_policy();
    list <Policy_Type *>::iterator iter;
    map <uint16_t, Mikey_Payload_SP*> existingSPpayloads;
    map <uint16_t, Mikey_Payload_SP*>::iterator mapiter;
    for( iter = (*policy).begin(); iter != (*policy).end()  ; iter++ )
    {
        mapiter = existingSPpayloads.find(comp);
        if (mapiter == existingSPpayloads.end())
        {
            existingSPpayloads.insert( pair<int, Mikey_Payload_SP*>(comp, PSP = new Mikey_Payload_SP((*iter)->policy_no, (*iter)->prot_type)));
            add_payload(PSP);
            PSP->add_mikey_policy_param((*iter)->policy_type, (*iter)->length, (*iter)->value);
        }
        else
            (mapiter->second)->add_mikey_policy_param((*iter)->policy_type, (*iter)->length, (*iter)->value);
    }
    existingSPpayloads.empty();
#undef comp
}

void Mikey_Payloads::add_policy_to_ka(Key_Agreement * ka)
{
#define SP dynamic_cast<Mikey_Payload_SP *>(*i)
    // Adding policy to ka
    int policy_i, policy_j;
    Mikey_Policy_Param * PParam;
    SRef<Mikey_Payload*> i;
    while ( 1 )
    {
        i = extract_payload( MIKEYPAYLOAD_SP_PAYLOAD_TYPE );
        if( i.is_null() )
        {
            break;
        }
        policy_i = 0;
        policy_j = 0;
        while (policy_i < SP->no_of_policy_param())
        {
            if((PParam = SP->get_parameter_type(policy_j++)) != NULL )
            {
                assert (policy_j-1 == PParam->type);
                ka->set_policy_param_type( SP-> policy_no, SP-> prot_type, PParam->type, PParam->length, PParam->value);
                policy_i++;
            }
        }
        payloads.remove( i );
    }
#undef SP
}

SRef<Mikey_Message *> Mikey_Message::parse_response( Key_Agreement  * ka )
{
    throw Mikey_Exception_Unimplemented( "parseResponse not implemented" );
}

void Mikey_Message::set_offer( Key_Agreement * ka )
{
    throw Mikey_Exception_Unimplemented( "setOffer not implemented" );
}

SRef<Mikey_Message *> Mikey_Message::build_response( Key_Agreement * ka )
{
    throw Mikey_Exception_Unimplemented( "buildResponse not implemented" );
}

bool Mikey_Message::authenticate( Key_Agreement  * ka )
{
    throw Mikey_Exception_Unimplemented( "authenticate not implemented" );
}

bool Mikey_Message::is_initiator_message() const
{
    return false;
}

bool Mikey_Message::is_responder_message() const
{
    return false;
}

int32_t Mikey_Message::key_agreement_type() const
{
    throw Mikey_Exception_Unimplemented( "Unimplemented type of MIKEY message" );
}


bool Mikey_Payloads::derive_transp_keys( Key_Agreement_PSK* ka, byte_t*& encrKey, byte_t *& iv, unsigned int& encrKeyLength,
                                         int encrAlg, int macAlg, uint64_t t, Mikey_Message* errorMessage )
{
    byte_t* authKey = NULL;
    bool error = false;
    unsigned int authKeyLength = 0;
    int i;

    encrKey = NULL;
    iv = NULL;
    encrKeyLength = 0;

    switch( encrAlg )
    {
    case MIKEY_ENCR_AES_CM_128:
    {
        byte_t saltKey[14];
        encrKeyLength = 16;
        encrKey = new byte_t[ encrKeyLength ];
        ka->gen_transp_encr_key(encrKey, encrKeyLength);
        ka->gen_transp_salt_key(saltKey, sizeof(saltKey));
        iv = new byte_t[ encrKeyLength ];
        iv[0] = saltKey[0];
        iv[1] = saltKey[1];
        for( i = 2; i < 6; i++ )
        {
            iv[i] = saltKey[i] ^ (ka->csb_id() >> (5-i)*8) & 0xFF;
        }

        for( i = 6; i < 14; i++ )
        {
            iv[i] = (byte_t)(saltKey[i] ^ (t >> (13-i)*8) & 0xFF);
        }
        iv[14] = 0x00;
        iv[15] = 0x00;
        break;
    }
    case MIKEY_ENCR_NULL:
        break;
    case MIKEY_ENCR_AES_KW_128:
        //TODO
    default:
        error = true;
        if( errorMessage )
        {
            errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_EA ) );
        }
        else
        {
            throw Mikey_Exception( "Unknown encryption algorithm" );
        }
    }
    switch( macAlg )
    {
    case MIKEY_MAC_HMAC_SHA1_160:
        authKeyLength = 20;
        authKey = new byte_t[ authKeyLength ];
        ka->gen_transp_auth_key(authKey, authKeyLength);
        break;
    case MIKEY_MAC_NULL:
        authKey = NULL;
        break;
    default:
        error = true;
        if( errorMessage )
        {
            errorMessage->add_payload( new Mikey_Payload_ERR( MIKEY_ERR_TYPE_INVALID_HA ) );
        }
        else
        {
            throw Mikey_Exception( "Unknown MAC algorithm" );
        }
    }

    ka->mac_alg = macAlg;
    if( ka->auth_key )
    {
        delete[] ka->auth_key;
    }
    ka->auth_key = authKey;
    ka->auth_key_length = authKeyLength;
    return !error;
}

void Mikey_Payloads::add_certificate_payloads( SRef<Certificate_Chain *> certChain )
{
    if( certChain.is_null() )
    {
        cerr << "No Certificates" << endl;
        return;
    }

    certChain->lock();
    certChain->init_index();
    SRef<Certificate*> cert = certChain->get_next();
    while( ! cert.is_null() )
    {
        SRef<Mikey_Payload*> payload = new Mikey_Payload_CERT( MIKEYPAYLOAD_CERT_TYPE_X509V3SIGN, cert);
        add_payload( payload );
        cert = certChain->get_next();
    }
    certChain->unlock();
}

SRef<Certificate_Chain*> Mikey_Payloads::extract_certificate_chain() const
{
    SRef<Certificate_Chain *> peerChain;

    /* Try to find the Certificate chain in the message */
    list<SRef<Mikey_Payload*> >::const_iterator i;
    list<SRef<Mikey_Payload*> >::const_iterator last = last_payload();

    for( i = first_payload(); i != last; i++ )
    {
        SRef<Mikey_Payload*> payload = *i;

        if( payload->payload_type() != MIKEYPAYLOAD_CERT_PAYLOAD_TYPE )
            continue;

        Mikey_Payload_CERT * certPayload = dynamic_cast<Mikey_Payload_CERT*>(*payload);
        SRef<Certificate*> peerCert = Certificate::load( certPayload->cert_data(), certPayload->cert_length() );

        if( peerChain.is_null() )
        {
            peerChain = Certificate_Chain::create();
        }
        // 		cerr << "Add Certificate: " << peerCert->get_name() << endl;
        peerChain->add_certificate( peerCert );
    }
    return peerChain;
}

bool Mikey_Payloads::verify_signature( SRef<Certificate*> cert, bool addIdsAndT )
{
    SRef<Mikey_Payload*> payload =
            extract_payload(MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE);

    if( !payload ){
        throw Mikey_Exception_Message_Content( "No SIGN payload" );
    }

    Mikey_Payload_SIGN* sig = dynamic_cast<Mikey_Payload_SIGN*>(*payload);
    vector<byte_t> signData;

    signData = build_sign_data( sig->sig_length(), addIdsAndT );

    int res = cert->verif_sign( &signData.front(), (int)signData.size(),
                               sig->sig_data(),
                               sig->sig_length() );
    return res > 0;
}

bool Mikey_Payloads::verify_kemac( Key_Agreement_PSK* ka, bool kemacOnly )
{
    int macAlg;
    byte_t * receivedMac;
    byte_t * macInput;
    unsigned int macInputLength;

    SRef<Mikey_Payload*> payload =
            extract_payload(MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE);

    if( !payload ){
        throw Mikey_Exception_Message_Content("No KEMAC payload");
    }

    Mikey_Payload_KEMAC * kemac;

    kemac = dynamic_cast<Mikey_Payload_KEMAC *>(*payload);

    macAlg = kemac->mac_alg();
    receivedMac = kemac->mac_data();

    if( kemacOnly )
    {
        macInputLength = kemac->length();
        macInput = new byte_t[macInputLength];
        kemac->write_data( macInput, macInputLength );
        macInput[0] = MIKEYPAYLOAD_LAST_PAYLOAD;
    }
    else{
        macInputLength = raw_message_length();
        macInput = new byte_t[macInputLength];
        memcpy( macInput, raw_message_data(), raw_message_length() );
    }

    macInputLength -= 20; // Subtract mac data
    bool ret = verify_mac( ka, macAlg, receivedMac, macInput, macInputLength );

    delete[] macInput;
    return ret;
}

bool Mikey_Payloads::verifyV( Key_Agreement_PSK* ka )
{
    int macAlg;
    byte_t * receivedMac;
    byte_t * macInput;
    unsigned int macInputLength;
    Mikey_Payload_V * v;
    uint64_t t_sent = ka->tsent();
    SRef<Mikey_Payload*> payload =
            extract_payload(MIKEYPAYLOAD_V_PAYLOAD_TYPE);

    if( !payload ){
        throw Mikey_Exception_Message_Content("No V payload");
    }

    v = dynamic_cast<Mikey_Payload_V*>(*payload);

    macAlg = v->mac_alg();
    receivedMac = v->ver_data();
    // macInput = raw_messsage without mac / sent_t
    macInputLength = raw_message_length() - 20 + 8;
    macInput = new byte_t[macInputLength];
    memcpy( macInput, raw_message_data(), raw_message_length() - 20 );

    for( int i = 0; i < 8; i++ )
    {
        macInput[ macInputLength - i - 1 ] =
                (byte_t)((t_sent >> (i*8))&0xFF);
    }

    bool ret = verify_mac( ka, macAlg, receivedMac, macInput, macInputLength );

    delete[] macInput;
    return ret;
}

bool Mikey_Payloads::verify_mac( Key_Agreement_PSK* ka, int macAlg, const byte_t* receivedMac, const byte_t* macInput, unsigned int macInputLength ) const
{
    byte_t authKey[20];
    byte_t computedMac[20];
    unsigned int computedMacLength;

    switch( macAlg )
    {
    case MIKEY_MAC_HMAC_SHA1_160:
        ka->gen_transp_auth_key( authKey, 20 );

        hmac_sha1( authKey, 20,
                   macInput,
                   macInputLength,
                   computedMac, &computedMacLength );

        for( int i = 0; i < 20; i++ )
        {
            if( computedMac[i] != receivedMac[i] )
            {
                ka->set_auth_error( "MAC mismatch." );
                return false;
            }
        }
        return true;
    case MIKEY_MAC_NULL:
        return true;
    default:
        throw Mikey_Exception( "Unknown MAC algorithm" );
    }
}

void Mikey_Payloads::add_pke_kemac( Key_Agreement_PKE* ka, int encrAlg, int macAlg )
{
    // Derive the transport keys from the env_key:
    byte_t* encrKey = NULL;
    byte_t* iv = NULL;
    unsigned int encrKeyLength = 0;

    derive_transp_keys( ka, encrKey, iv, encrKeyLength,
                        encrAlg, macAlg, ka->tsent(),
                        NULL );

    //adding KEMAC payload
    Mikey_Payloads* subPayloads = new Mikey_Payloads();
    Mikey_Payload_Key_Data* keydata =
            new Mikey_Payload_Key_Data(KEYDATA_TYPE_TGK, ka->tgk(),
                                       ka->tgk_length(), ka->key_validity());

    subPayloads->add_id( ka->uri() );
    subPayloads->add_payload( keydata );
    keydata = NULL;

    unsigned int rawKeyDataLength = subPayloads->raw_message_length();
    byte_t* rawKeyData = new byte_t[ rawKeyDataLength ];
    memcpy( rawKeyData, subPayloads->raw_message_data(), rawKeyDataLength );

    add_kemac_payload(rawKeyData, rawKeyDataLength,
                      encrKey, iv, ka->auth_key, encrAlg, macAlg, true );

    if( encrKey )
    {
        delete[] encrKey;
        encrKey = NULL;
    }

    if( iv )
    {
        delete[] iv;
        iv = NULL;
    }

    delete subPayloads;
    subPayloads = NULL;

    delete [] rawKeyData;
    rawKeyData = NULL;

    //adding PKE payload
    SRef<Certificate*> certResponder =
            ka->peer_certificate_chain()->get_first();

    byte_t* env_key = ka->get_envelope_key();
    int encEnvKeyLength = 8192; // TODO autodetect?
    unsigned char* encEnvKey = new unsigned char[ encEnvKeyLength ];

    if( !certResponder->public_encrypt( env_key, ka->get_envelope_key_length(),
                                       encEnvKey, &encEnvKeyLength ) )
    {
        throw Mikey_Exception( "PKE encryption of envelope key failed" );
    }

    add_payload(new Mikey_Payload_PKE(2, encEnvKey, encEnvKeyLength));

    delete [] encEnvKey;
    encEnvKey = NULL;
}

bool Mikey_Payloads::extract_pke_env_key( Key_Agreement_PKE* ka ) const
{
    SRef<const Mikey_Payload*> payloadPke = extract_payload( MIKEYPAYLOAD_PKE_PAYLOAD_TYPE );
    if( !payloadPke )
    {
        throw Mikey_Exception( "PKE init did not contain PKE payload" );
    }

    const Mikey_Payload_PKE *pke = dynamic_cast<const Mikey_Payload_PKE*>( *payloadPke );

    if( !pke )
        throw Mikey_Exception( "PKE init did not contain PKE payload" );

    SRef<Certificate*> cert = ka->certificate_chain()->get_first();
    int envKeyLength = pke->data_length();
    byte_t *envKey = new byte_t[ envKeyLength ];

    if( !cert->private_decrypt( pke->data(), pke->data_length(), envKey, &envKeyLength ) )
    {
        throw Mikey_Exception( "Decryption of envelope key failed" );
    }

    ka->set_envelope_key( envKey, envKeyLength );

    delete[] envKey;
    envKey = NULL;
    return true;
}

void Mikey_Payloads::add_id( const std::string &theId )
{
    int type = MIKEYPAYLOAD_ID_TYPE_URI;
    string id = theId;

    if( id.substr( 0, 4 ) == "nai:" )
    {
        type = MIKEYPAYLOAD_ID_TYPE_NAI;
        id = id.substr( 4 );
    }

    Mikey_Payload_ID* initId = new Mikey_Payload_ID( type, (int)id.size(), (byte_t*)id.c_str() );
    add_payload( initId );
}

const Mikey_Payload_ID* Mikey_Payloads::extract_id( int index ) const
{
    const Mikey_Payload_ID *id = NULL;
    list<SRef<Mikey_Payload*> >::const_iterator i;
    list<SRef<Mikey_Payload*> >::const_iterator last = last_payload();
    int j;

    for( i = first_payload(), j = 0; i != last; i++ )
    {
        SRef<Mikey_Payload*> payload = *i;

        if( payload->payload_type() == MIKEYPAYLOAD_ID_PAYLOAD_TYPE )
        {
            if( j == index )
            {
                id = dynamic_cast<const Mikey_Payload_ID*>(*payload);
                break;
            }

            j++;
        }
    }
    return id;
}

std::string Mikey_Payloads::extract_id_str( int index ) const
{
    const Mikey_Payload_ID *id = extract_id( index );

    if( !id )
        return "";

    string idData = string( (const char*)id->id_data(), id->id_length() );
    string idStr;

    switch( id->id_type() )
    {
    case MIKEYPAYLOAD_ID_TYPE_NAI:
        idStr = "nai:" + idData;
        break;

    case MIKEYPAYLOAD_ID_TYPE_URI:
        idStr = idData;
        break;

    default:
        return "";
    }
    return idStr;
}
std::vector<byte_t> Mikey_Payloads::extract_id_vec( int index ) const
{
    const Mikey_Payload_ID *id = extract_id( index );
    vector<byte_t> result;

    if( !id )
        return result;

    result.resize( id->id_length() );
    memcpy( &result.front(), id->id_data(), id->id_length() );
    return result;
}
