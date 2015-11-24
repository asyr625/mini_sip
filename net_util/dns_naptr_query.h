#ifndef DNS_NAPTR_QUERY_H
#define DNS_NAPTR_QUERY_H
#include <list>
#include "sobject.h"

class Dns_Naptr_Query : public SObject
{
public:
    virtual ~Dns_Naptr_Query();

    static Dns_Naptr_Query *create();

    enum ResultType
    {
        NONE = 0,
        SRV,
        ADDR,
        URI
    };

    void set_enum_domains(std::list<std::string> &enumdomains);

    virtual void set_accept( const std::list<std::string> &acceptServices ) = 0;
    virtual ResultType get_result_type() const = 0;
    virtual const std::string &get_result() const = 0;
    virtual const std::string &get_service() const = 0;

    virtual bool resolve( const std::string &domain, const std::string &target ) = 0;
    bool resolve_isn( const std::string &isn );

    bool resolve_enum( const std::string &e164 );

    bool resolve_sip( const std::string &domain );
    bool resolve_sips( const std::string &domain );
protected:
    Dns_Naptr_Query();

private:
    bool resolve_common( const std::string &domain, const std::string &target );

    std::list<std::string> _enum_domains;
};

#endif // DNS_NAPTR_QUERY_H
