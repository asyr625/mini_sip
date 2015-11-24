#ifndef SIP_HEADER_H
#define SIP_HEADER_H

#include <map>

#include "sobject.h"
#include "mini_list.h"
#include "my_types.h"

#define SIP_HEADER_TYPE_ACCEPT			0
#define SIP_HEADER_TYPE_AUTHORIZATION		1
#define SIP_HEADER_TYPE_CALLID			2
#define SIP_HEADER_TYPE_CONTACT			3
#define SIP_HEADER_TYPE_CONTENTLENGTH		4
#define SIP_HEADER_TYPE_CONTENTTYPE		5
#define SIP_HEADER_TYPE_CSEQ			6
#define SIP_HEADER_TYPE_EVENT			7
#define SIP_HEADER_TYPE_EXPIRES			8
#define SIP_HEADER_TYPE_FROM			9
#define SIP_HEADER_TYPE_MAXFORWARDS		10
#define SIP_HEADER_TYPE_PROXYAUTHENTICATE	11
#define SIP_HEADER_TYPE_PROXYAUTHORIZATION	12
#define SIP_HEADER_TYPE_RECORDROUTE		13
#define SIP_HEADER_TYPE_ROUTE			14
#define SIP_HEADER_TYPE_SUBJECT			15
#define SIP_HEADER_TYPE_TO			16
#define SIP_HEADER_TYPE_USERAGENT		17
#define SIP_HEADER_TYPE_VIA			18
#define SIP_HEADER_TYPE_UNKNOWN                 19
#define SIP_HEADER_TYPE_ACCEPTCONTACT		20
#define SIP_HEADER_TYPE_WARNING			21
#define SIP_HEADER_TYPE_REFERTO			22
#define SIP_HEADER_TYPE_WWWAUTHENTICATE		25
#define SIP_HEADER_TYPE_SUPPORTED		26
#define SIP_HEADER_TYPE_UNSUPPORTED		27
#define SIP_HEADER_TYPE_REQUIRE			28
#define SIP_HEADER_TYPE_RACK			29
#define SIP_HEADER_TYPE_RSEQ			30
#define SIP_HEADER_TYPE_ALLOWEVENTS		31
#define SIP_HEADER_TYPE_SUBSCRIPTIONSTATE	32
#define SIP_HEADER_TYPE_ALLOW			33
#define SIP_HEADER_TYPE_SNAKESM			34
#define SIP_HEADER_TYPE_AUTHENTICATIONINFO      35
#define SIP_HEADER_TYPE_SESSION_EXPIRES         36


class Sip_Header_Value;

typedef SRef<Sip_Header_Value*> (*Sip_Header_Factory_Func_Ptr)(const std::string& buf);

class Sip_Header_Factories
{
public:
    void add_factory(std::string headerType, Sip_Header_Factory_Func_Ptr f);
    Sip_Header_Factory_Func_Ptr get_factory(std::string headerType) const;
private:
    std::map<std::string, Sip_Header_Factory_Func_Ptr> _factories;
};


class Sip_Header_Parameter : public SObject
{
public:
    Sip_Header_Parameter(std::string parse_from);
    Sip_Header_Parameter(std::string key, std::string value, bool hasEqual);
    std::string get_mem_object_type() const {return "SipHeaderParameter";}
    std::string get_key() const;
    std::string get_value() const;
    void set_value(std::string v);
    std::string get_string() const;
private:
    std::string _key;
    std::string _value;
    bool        _has_equal;
};


class Sip_Header_Value : public SObject
{
public:
    Sip_Header_Value(int type, const std::string &hName);
    virtual std::string get_string() const = 0;

    int get_type(){return _type;}

    void set_parameter(std::string key, std::string val);

    void add_parameter(SRef<Sip_Header_Parameter*> p);

    bool has_parameter(const std::string &key) const;

    std::string get_parameter(std::string key) const;

    void remove_parameter(std::string key);

    std::string get_string_with_parameters() const ;

    const std::string &header_name;
private:
    virtual char get_first_parameter_separator() const { return ';'; }
    virtual char get_parameter_separator() const { return ';'; }

    int _type;
    Mini_List<SRef<Sip_Header_Parameter*> > _parameters;
};

class Sip_Header : public SObject
{
public:
    static Sip_Header_Factories header_factories;
    Sip_Header(SRef<Sip_Header_Value*> value);
    virtual ~Sip_Header();

    std::string get_string() const ;
    void add_header_value(SRef<Sip_Header_Value*> v);

    virtual std::string get_mem_object_type() const { return "SipHeader"; }

    int32_t get_type() const;
    int get_no_values() const;
    SRef<Sip_Header_Value *> get_header_value(int i) const;
    void remove_header_value(int i);

    static SRef<Sip_Header *> parse_header(const std::string &line);

private:
    int _type;
    std::string _header_name;
    Mini_List<SRef<Sip_Header_Value*> > _header_values;
};

#endif // SIP_HEADER_H
