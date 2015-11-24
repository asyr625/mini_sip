#ifndef STUN_MESSAGE_H
#define STUN_MESSAGE_H
#include <list>

#include <string>
#include <stun_attribute.h>

class Message_Header
{
public:
    Message_Header(int type);

    void set_payload_length(int len);
    int get_payload_length();
    void set_type(int type);
    void set_transaction_id(unsigned char *id);
    int get_header_length(){return 20;}

    int get_data(unsigned char*buf);

    unsigned char transaction_id[16];
private:
    uint16_t message_type;
    uint16_t message_length;
};

class Stun_Message
{
public:
    static const int BINDING_REQUEST;
    static const int BINDING_RESPONSE;
    static const int BINDING_ERROR_RESPONSE;
    static const int SHARED_SECRET_REQUEST;
    static const int SHARES_SECRET_RESPONSE;
    static const int SHARED_SECRET_ERROR_RESPONSE;

    Stun_Message(unsigned char*buf, int length);
    Stun_Message(int type);
    virtual ~Stun_Message();

    bool same_transaction_id(Stun_Message &msg);

    void add_attribute(Stun_Attribute *a);

    unsigned char* get_message_data(int &retLength); //the user is responsible for deleteing the data
    void send_message(int fd);
    Stun_Attribute *get_attribute(int type);

protected:
    void parse_attributes(unsigned char *data, int length);

    Message_Header header;
    std::list<Stun_Attribute *> attributes;
};

#endif // STUN_MESSAGE_H
