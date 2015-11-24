#ifndef MIKEY_PAYLOAD_SP_H
#define MIKEY_PAYLOAD_SP_H

#include <list>
#include "sobject.h"
#include "mikey_payload.h"

#define MIKEYPAYLOAD_SP_PAYLOAD_TYPE 10
//
// Constants for SRTP
//
#define MIKEY_PROTO_SRTP		0
//SRTP encryption algorithms (RFC 3830)
#define MIKEY_SRTP_EALG_NULL		0
#define MIKEY_SRTP_EALG_AESCM		1
#define MIKEY_SRTP_EALG_AESF8		2
//SRTP authentication algorithms (RFC 3830)
#define MIKEY_SRTP_AALG_NULL		0
#define MIKEY_SRTP_AALG_SHA1HMAC	1
//SRTP pseudo-random (RFC 3830)
#define MIKEY_SRTP_PRF_AESCM		0
//FEC order (RFC 3830)
#define MIKEY_FEC_ORDER_FEC_SRTP	0
// Policy Param type for SRTP (RFC 3830)
#define MIKEY_SRTP_EALG			0
#define MIKEY_SRTP_EKEYL		1
#define MIKEY_SRTP_AALG			2
#define MIKEY_SRTP_AKEYL		3
#define MIKEY_SRTP_SALTKEYL		4
#define MIKEY_SRTP_PRF			5
#define MIKEY_SRTP_KEY_DERRATE		6
#define MIKEY_SRTP_ENCR_ON_OFF		7
#define MIKEY_SRTCP_ENCR_ON_OFF		8
#define MIKEY_SRTP_FEC_ORDER		9
#define MIKEY_SRTP_AUTH_ON_OFF 		10
#define MIKEY_SRTP_AUTH_TAGL		11
#define MIKEY_SRTP_PREFIX		12
//
// Constants for IPSEC
//
#define MIKEY_PROTO_IPSEC4		7
//IPSEC encryption algorithms (RFC 2367)
#define MIKEY_IPSEC_EALG_NONE		0
#define MIKEY_IPSEC_EALG_DESCBC		2
#define MIKEY_IPSEC_EALG_3DESCBC	3
#define MIKEY_IPSEC_EALG_NULL		11
//IPSEC authentication algorithms (RFC 2367)
#define MIKEY_IPSEC_AALG_NONE		0
#define MIKEY_IPSEC_AALG_MD5HMAC	2
#define MIKEY_IPSEC_AALG_SHA1HMAC	3
// IPSEC SA type (RFC 2367)
#define MIKEY_IPSEC_SATYPE_UNSPEC	0
#define MIKEY_IPSEC_SATYPE_AH		2
#define MIKEY_IPSEC_SATYPE_ESP		3
// IPSEC SA type
#define MIKEY_IPSEC_MODE_ANY		0
#define MIKEY_IPSEC_MODE_TRANSPORT	1
#define MIKEY_IPSEC_MODE_TUNNEL		2
// options defined for SA
#define MIKEY_IPSEC_SAFLAG_NONE		0x0000	 //i.e. new format.
#define MIKEY_IPSEC_SAFLAG_OLD		0x0001	 //old format.
#define MIKEY_IPSEC_SAFLAG_IV4B		0x0010	 //IV length of 4 bytes in use
#define MIKEY_IPSEC_SAFLAG_DERIV	0x0020	 //DES derived
#define MIKEY_IPSEC_SAFLAG_CYCSEQ	0x0040	 //allowing to cyclic sequence.
//three of followings are exclusive flags each them
#define MIKEY_IPSEC_SAFLAG_PSEQ		0x0000	 //sequencial padding for ESP
#define MIKEY_IPSEC_SAFLAG_PRAND	0x0100	 //random padding for ESP
#define MIKEY_IPSEC_SAFLAG_PZERO	0x0200	 //zero padding for ESP
#define MIKEY_IPSEC_SAFLAG_PMASK	0x0300	 //mask for padding flag
#define MIKEY_IPSEC_SAFLAG_RAWCPI	0x0080	 //use well known CPI (IPComp)
// Policy Param type for IPSEC
#define MIKEY_IPSEC_SATYPE		0
#define MIKEY_IPSEC_MODE		1
#define MIKEY_IPSEC_SAFLAG		2
#define MIKEY_IPSEC_EALG		3
#define MIKEY_IPSEC_EKEYL		4
#define MIKEY_IPSEC_AALG		5
#define MIKEY_IPSEC_AKEYL		6

class Mikey_Policy_Param
{
public:
    Mikey_Policy_Param( uint8_t type, uint8_t length, byte_t * value );
    ~Mikey_Policy_Param();
    uint8_t type;	// As defined above
    uint8_t length;	// Length of value in bytes
    byte_t * value; // type value
};

class Mikey_Payload_SP : public Mikey_Payload
{
public:
    Mikey_Payload_SP(uint8_t policy_no, uint8_t prot_type);
    Mikey_Payload_SP(byte_t *start, int lengthLimit);

    ~Mikey_Payload_SP();
    Mikey_Policy_Param * get_parameter_type(uint8_t type);
    virtual void write_data(byte_t *start, int expectedLength);

    virtual int length();

    int no_of_policy_param();
    std::string debug_dump();

    uint8_t policy_no;
    uint8_t prot_type;

    void add_mikey_policy_param( uint8_t type, uint8_t length, byte_t * value);
private:
    void delete_mikey_policy_param(uint8_t type);

    uint16_t policy_param_length;
    std::list<Mikey_Policy_Param *> param;
};

#endif // MIKEY_PAYLOAD_SP_H
