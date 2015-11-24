#ifndef ZRTP_HOST_BRIDGE_MINISIP_H
#define ZRTP_HOST_BRIDGE_MINISIP_H


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "state_machine.h"
#include "message_router.h"
#include "sip_smcommand.h"

#include "srtp_packet.h"
#include "crypto_context.h"

#include "ZRtp.h"
#include "ZrtpCallback.h"

class Realtime_Media_Stream_Receiver;
class Realtime_Media_Stream_Sender;

#ifdef ZRTP_SUPPORT

class Zrtp_Host_Bridge_Minisip : public State_Machine<Sip_SMCommand,std::string>, public ZrtpCallback
{
public:
    virtual std::string get_mem_object_type() const { return "ZrtpHostBridgeMinisip";}
    static int32_t initialize(SRef<Timeout_Provider<std::string,
                              SRef<State_Machine<Sip_SMCommand,std::string>*> > *> tp,
                              const char *zidFilename =NULL);

    Zrtp_Host_Bridge_Minisip(std::string id, SRef<Command_Receiver*> callback);
    ~Zrtp_Host_Bridge_Minisip();


    void start();
    void stop();

    void setReceiver(SRef<Realtime_Media_Stream_Receiver *> r);
    void setSsrcReceiver(uint32_t ssrc)             { receiverSsrc = ssrc; }
    uint32_t getSsrcReceiver()                      { return receiverSsrc; }

    void setSender(SRef<Realtime_Media_Stream_Sender *> s);
    void setSsrcSender(uint32_t ssrc)               { senderSsrc = ssrc; }
    uint32_t getSsrcSender()                        { return senderSsrc; }

    bool isSecureState();

    void setCallId(std::string id)                  { callId = id; }

    void setRemoteAddress(SRef<IPAddress *> ra) { remoteAddress = ra; }
    SRef<IPAddress *> getRemoteAddress() { return remoteAddress; }
    int32_t processPacket(SRef<SRtp_Packet *> packet);

    void handleTimeout(const std::string & /* c */ )
    {
        if (zrtpEngine != NULL) {
            zrtpEngine->processTimeout();
        }
    }

    int32_t sendDataRTP(const unsigned char* data, int32_t length);

    int32_t sendDataSRTP(const unsigned char* dataHeader, int32_t lengthHeader,
                         char *dataContent, int32_t lengthContent);

    int32_t activateTimer(int32_t time)
    {
        std::string s("ZRTP");
        request_timeout(time, s);
        return 1;
    }

    int32_t cancelTimer()
    {
        std::string s("ZRTP");
        cancelTimeout(s);
        return 1;
    }

    void sendInfo(MessageSeverity severity, const char* msg)
    {
        fprintf(stderr, "Severity: %d - %s\n", severity, msg);
    }

    void handleGoClear()
    {
        fprintf(stderr, "Need to process a GoClear message!");
    }

    void srtpSecretsReady(SrtpSecret_t* secrets, EnableSecurity part);

    virtual void srtpSecretsOn(const char* c, const char* s);

    void srtpSecretsOff(EnableSecurity part);

    void zrtpNegotiationFailed(MessageSeverity severity, const char* msg);

    void zrtpNotSuppOther();

    void rtpSessionError();

    void setZfoneDeadBeef(int8_t onOff)  { zfoneDeadBeef = onOff; }

    int8_t getZfoneDeadBeef()           {return zfoneDeadBeef; }

    uint16_t getZrtpSendSeqNo()         { return senderZrtpSeqNo++; }

    uint32_t getZrtpSendSsrc()          { return senderZrtpSsrc; }

    SRef<Crypto_Context *> newCrypto_ContextForRecvSSRC(uint32_t ssrc, int roc, uint16_t seq,
                                                      int64_t keyDerivRate);

    bool isZrtpPacket(SRef<SRtp_Packet *> packet);

private:
    ZRtp *zrtpEngine;
    SrtpSecret_t secret;
    int32_t secureParts;

    SRef<IPAddress *> remoteAddress;

    SRef<Realtime_Media_Stream_Receiver *> rStream;
    uint32_t receiverSsrc;
    uint32_t receiverSecure;
    uint16_t receiverSeqNo;

    SRef<Realtime_Media_Stream_Sender *> sStream;
    uint32_t senderSsrc;
    uint32_t senderSecure;

    bool enableZrtp;

    uint32_t recvZrtpSsrc;
    uint16_t recvZrtpSeqNo;
    SRef<Crypto_Context *> recvCrypto_Context;

    uint32_t senderZrtpSsrc;
    uint16_t senderZrtpSeqNo;
    SRef<Crypto_Context *> senderCrypto_Context;

    std::string callId;

    SRef<Command_Receiver*> messageRouterCallback;
    int8_t zfoneDeadBeef;
};

#endif

#endif // ZRTP_HOST_BRIDGE_MINISIP_H
