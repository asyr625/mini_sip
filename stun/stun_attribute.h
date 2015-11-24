#ifndef STUN_ATTRIBUTE_H
#define STUN_ATTRIBUTE_H

#include "my_types.h"
#include "string_utils.h"

class Stun_Attribute
{
public:
    static const int MAPPED_ADDRESS;
    static const int RESPONSE_ADDRESS;
    static const int CHANGE_REQUEST;
    static const int SOURCE_ADDRESS;
    static const int CHANGED_ADDRESS;
    static const int USERNAME;
    static const int PASSWORD;
    static const int MESSAGE_INTEGRITY;
    static const int ERROR_CODE;
    static const int UNKNOWN_ATTRIBUTES;
    static const int REFLECTED_FROM;

    Stun_Attribute(int type);
    virtual ~Stun_Attribute();

    virtual int get_type();
    virtual int get_value_length() = 0;
    virtual std::string get_desc() = 0;
    virtual int get_message_data_tlv(unsigned char* buf);

    static Stun_Attribute *parse_attribute(
            unsigned char *data,
            int maxLength,
            int &retParsedLength //return parameter - how long the parsed attr is
            );
protected:
    virtual int get_value(unsigned char *buf) = 0;
private:
    uint16_t type;
};


class Stun_Attribute_Address : public Stun_Attribute
{
public:
    Stun_Attribute_Address(int type, uint16_t port, char *addr);
    Stun_Attribute_Address(int type, unsigned char *data, int length);

    virtual std::string get_desc();
    uint32_t get_binary_ip();
    uint16_t get_port();
protected:
    virtual int get_value(unsigned char *buf);
    virtual int get_value_length();
private:
    unsigned char family;
    uint16_t port;
    uint32_t address;
};

class Stun_Attribute_Mapped_Address : public Stun_Attribute_Address
{
public:
    Stun_Attribute_Mapped_Address(char *addr, uint16_t port);
    Stun_Attribute_Mapped_Address(int length, unsigned char *data);
};

class Stun_Attribute_Response_Address : public Stun_Attribute_Address
{
public:
    Stun_Attribute_Response_Address(char *addr, uint16_t port);
    Stun_Attribute_Response_Address(int length, unsigned char *data);
};


class Stun_Attribute_Changed_Address : public Stun_Attribute_Address
{
public:
    Stun_Attribute_Changed_Address(char *addr, uint16_t port);
    Stun_Attribute_Changed_Address(int length, unsigned char *data);
};


class Stun_Attribute_Source_Address : public Stun_Attribute_Address
{
public:
    Stun_Attribute_Source_Address(char *addr, uint16_t port);
    Stun_Attribute_Source_Address(int length, unsigned char *data);
};

class Stun_Attribute_Change_Request : public Stun_Attribute
{
public:
    Stun_Attribute_Change_Request(bool changeIP, bool changePort);
    Stun_Attribute_Change_Request(unsigned char *data, int length);
    virtual std::string get_desc()
    {
        return std::string("type: CHANGE REQUEST; changeIP: ") + itoa(change_ip) + "; changePort: " + itoa(change_port);
    }
protected:
    virtual int get_value(unsigned char *buf);
    virtual int get_value_length();
private:
    bool change_ip;
    bool change_port;
};

class Stun_Attribute_String : public Stun_Attribute
{
public:
    Stun_Attribute_String(int type, char *str, int strlen);
    Stun_Attribute_String(int type, int length, unsigned char *data);
    ~Stun_Attribute_String();
protected:
    virtual int get_value(unsigned char *buf);
    virtual int get_value_length();
private:
    char *str;
    int length;
};

class Stun_Attribute_Username : public Stun_Attribute_String
{
public:
    Stun_Attribute_Username(char *username, int length);
    Stun_Attribute_Username(int length, unsigned char* data);
    virtual std::string get_desc()
    {
        return "USERNAME";
    }
};

class Stun_Attribute_Password : public Stun_Attribute_String
{
public:
    Stun_Attribute_Password(char *password, int length);
    Stun_Attribute_Password(int length, unsigned char* data);
    virtual std::string get_desc()
    {
        return "PASSWORD";
    }
};

class Stun_Attribute_Error_Code : public Stun_Attribute
{
public:
    Stun_Attribute_Error_Code(char *msg, int errorCode);
    Stun_Attribute_Error_Code(int length, unsigned char* data);
    ~Stun_Attribute_Error_Code();

    virtual int get_value(unsigned char *buf);
    virtual int get_value_length();
    virtual std::string get_desc()
    {
        return std::string("Error code: ")+itoa(errorCode)+" Message: "+message;
    }

private:
    int errorCode;
    char *message;
    int messageLength;
};

class Stun_Attribute_Reflected_From : public Stun_Attribute_Address
{
public:
    Stun_Attribute_Reflected_From(char *addr, uint16_t port);
    Stun_Attribute_Reflected_From(int length, unsigned char *data);
};

class Stun_Attribute_Unknown_Attributes : public Stun_Attribute
{
public:
    Stun_Attribute_Unknown_Attributes(uint16_t *addr, int n_attribs);
    Stun_Attribute_Unknown_Attributes(int length, unsigned char *data);

    ~Stun_Attribute_Unknown_Attributes();

    virtual int get_value(unsigned char *buf);
    virtual int get_value_length();

    virtual std::string get_desc()
    {
        return std::string("Unknown Attributes");
    }
private:
    uint16_t *attributes;
    int nAttributes;
};

#endif // STUN_ATTRIBUTE_H
