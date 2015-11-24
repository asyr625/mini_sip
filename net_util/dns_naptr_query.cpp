#include "dns_naptr_query.h"
#include "udns.h"

#include<list>
#ifdef HAVE_REGEX_H
#include<regex.h>
#endif
#include<algorithm>
#include<string.h>
#include<stdio.h>

using namespace std;

Dns_Naptr_Query::Dns_Naptr_Query()
{
    _enum_domains.push_back("e164.arpa");
    _enum_domains.push_back("e164.org");
}

Dns_Naptr_Query::~Dns_Naptr_Query()
{
}

static std::string num_domain( const std::string &str )
{
    std::string num = str;

    if( str.at( 0 ) == '+' )
        num.erase( 0, 1 );

    std::string::reverse_iterator i;
    std::string::reverse_iterator last = num.rend();

    std::string domain;

    for( i = num.rbegin(); i != last; i++ )
    {
        domain += *i;
        domain += '.';
    }

    return domain;
}

bool Dns_Naptr_Query::resolve_common( const std::string &domain, const std::string &target )
{
    std::list<std::string> acceptServices;
    acceptServices.push_back("E2U+SIP");
    acceptServices.push_back("SIP+E2U");

    set_accept( acceptServices );

    if( !resolve( domain, target ) )
        return false;

    return get_result_type() == Dns_Naptr_Query::URI;
}

void Dns_Naptr_Query::set_enum_domains( std::list<std::string> &enumdomains)
{

}

bool Dns_Naptr_Query::resolve_isn( const std::string &isn )
{
    static const std::string prefix = ".freenum.org";

    std::string::size_type pos;

    pos = isn.find('*');

    if(pos == std::string::npos)
        return false;

    std::string exten = isn.substr( 0, pos );
    std::string itad = isn.substr( pos + 1 );

    std::string domain = num_domain( exten ) + itad + prefix;

    return resolve_common( domain, isn );
}

bool Dns_Naptr_Query::resolve_enum( const std::string &e164 )
{
    std::list<std::string>::const_iterator i;

    for( i = _enum_domains.begin(); i != _enum_domains.end(); i++ )
    {
        std::string enumDomain = *i;

        std::string domain = num_domain( e164 ) + enumDomain;

        if( resolve_common( domain, e164 ) )
            return true;
    }

    return false;
}

bool Dns_Naptr_Query::resolve_sip( const std::string &domain )
{
    std::list<std::string> acceptServices;
    acceptServices.push_back("SIPS+D2T");
    acceptServices.push_back("SIP+D2T");
    acceptServices.push_back("SIP+D2U");

    set_accept( acceptServices );

    if( !resolve( domain, domain ) )
        return false;

    return get_result_type() == Dns_Naptr_Query::SRV;
}

bool Dns_Naptr_Query::resolve_sips( const std::string &domain )
{
    std::list<std::string> acceptServices;
    acceptServices.push_back("SIPS+D2T");

    set_accept( acceptServices );

    if( !resolve( domain, domain ) )
        return false;

    return get_result_type() == Dns_Naptr_Query::SRV;
}


typedef std::list<dns_naptr*> Naptr_List;

class Dns_Naptr_Query_Priv: public Dns_Naptr_Query
{
public:
    Dns_Naptr_Query_Priv();
    virtual ~Dns_Naptr_Query_Priv();

    virtual void set_accept( const std::list<std::string> &acceptServices );

    virtual ResultType get_result_type() const;
    virtual const std::string &get_result() const;
    virtual const std::string &get_service() const;

    virtual bool resolve( const std::string &domain,
                          const std::string &target );

private:
    bool calc_regexp( const std::string &regexp );
    bool process( const Naptr_List &lst );
    bool dump_naptr( dns_rr_naptr *naptr );

    dns_ctx *ctx;
    dns_rr_naptr *naptr;
    const std::list<std::string> *accept_services;
    std::string target;
    ResultType result_type;
    std::string result;
    std::string service;
};

Dns_Naptr_Query *Dns_Naptr_Query::create()
{
    return new Dns_Naptr_Query_Priv();
}


Dns_Naptr_Query_Priv::Dns_Naptr_Query_Priv()
    :naptr( NULL ), accept_services( NULL )
{
    ctx = dns_new(NULL);
    //   if (argc > 2)
    //     dns_add_serv(ctx, argv[2]);
    dns_open(ctx);
}

Dns_Naptr_Query_Priv::~Dns_Naptr_Query_Priv()
{
    if (ctx)
        dns_free(ctx);
    ctx=NULL;
}

void Dns_Naptr_Query_Priv::set_accept( const std::list<std::string> &theAcceptServices )
{
    accept_services = &theAcceptServices;
}


Dns_Naptr_Query::ResultType Dns_Naptr_Query_Priv::get_result_type() const
{
    return result_type;
}

const std::string &Dns_Naptr_Query_Priv::get_result() const
{
    return result;
}

const std::string &Dns_Naptr_Query_Priv::get_service() const
{
    return service;
}

#ifdef DEBUG_OUTPUT
void dump_srv(dns_rr_srv *srv)
{
    for (int i=0; i < srv->dnssrv_nrr; i++)
    {
        dns_srv *rr = &srv->dnssrv_srv[i];
        cerr << rr->priority << " "
             << rr->weight << " "
             << rr->port << " \""
             << rr->name << endl;
    }
}

void dump_naptr_entry(dns_naptr *rr)
{
    cerr << rr->order << " "
         << rr->preference << " \""
         << rr->flags << "\" \""
         << rr->service << "\" \""
         << rr->regexp << "\" "
         << rr->replacement << endl;
}

void dump_naptr_list(const Naptr_List &lst)
{
    Naptr_List::const_iterator last = lst.end();
    Naptr_List::const_iterator i;
    for(i = lst.begin(); i != last; i++)
    {
        dump_naptr_entry(*i);
    }
}
#endif

bool dns_naptr_pred( const dns_naptr* lhs, const dns_naptr* rhs )
{
    if( lhs->order != rhs->order )
    {
        bool res;
        res = lhs->order < rhs->order;
#ifdef DEBUG_OUTPUT
        cerr << "order " << res << endl;
#endif
        return res;
    }

    if( lhs->preference != rhs->preference )
    {
        bool res;
        res = lhs->preference < rhs->preference;
#ifdef DEBUG_OUTPUT
        cerr << "preference " << res << endl;
#endif
        return res;
    }

    return 0;
}

bool Dns_Naptr_Query_Priv::calc_regexp( const std::string &regexp )
{
#ifndef HAVE_POSIX_REGCOMP
    return false;
#else
    static const std::string back_slash = "\\";
    regex_t preg;
    regmatch_t pmatch[9];

    if( regexp.empty() )
        return false;

    memset(&preg, 0, sizeof(preg));
    memset(pmatch, 0, sizeof(pmatch));

    char c = regexp[0];
    std::string::size_type pos = regexp.find( c, 1 );
    std::string::size_type posFlags = regexp.find( c, pos + 1 );

    if( pos == std::string::npos || posFlags == std::string::npos )
    {
        //cerr << "Bad regexp" << endl;
        return false;
    }

    std::string pattern = regexp.substr( 1, pos - 1 );
    result = regexp.substr( pos + 1, posFlags - pos - 1 );
    std::string flags = regexp.substr( posFlags + 1,
                                       regexp.length() - posFlags - 1 );
    // TODO support regexp flags, ie. case flag "i"

#ifdef DEBUG_OUTPUT
    cerr << "Try Regex " << pattern << "," << result << "," << flags << endl;
#endif
    if( regcomp(&preg, pattern.c_str(), REG_EXTENDED) ){
        perror("regcomp");
        return false;
    }

    int res = regexec(&preg, target.c_str(),
                      sizeof(pmatch)/sizeof(pmatch[0]), pmatch, 0);
    if( res )
    {
        char buf[256]="";
        regerror(res, &preg, buf, sizeof(buf));
        //cerr << "Error " << buf << endl;
        return false;
    }

    for( int i = 1; i <= 9; i++ )
    {
        std::string::size_type so;
        std::string tag = back_slash + std::string( 1, '0' + i );

        if( pmatch[i].rm_so == -1 )
            continue;

        so = result.find( tag );

        if( so == std::string::npos )
            continue;

        std::string replacement = target.substr( pmatch[i].rm_so,
                                                 pmatch[i].rm_eo - pmatch[i].rm_so );

#ifdef DEBUG_OUTPUT
        cerr << "MATCH REGEX " << pmatch[i].rm_so << " " << pmatch[i].rm_eo << "," << replacement << endl;
        cerr << "result " << result << endl;
        int ins_len = replacement.length() - tag.length();
        cerr << "ins_len " << ins_len << endl;

        cerr << "before replace '" << result << "'" << endl;
#endif
        result.replace( so, tag.length(), replacement, 0, replacement.length() );
    }

    regfree(&preg);
#ifdef DEBUG_OUTPUT
    cerr << "MATCH REGEX " << result << endl;
#endif

    return true;
#endif	// HAVE_POSIX_REGCOMP
}


bool Dns_Naptr_Query_Priv::process( const Naptr_List &lst )
{
    Naptr_List::const_iterator last = lst.end();
    Naptr_List::const_iterator i;
    for(i = lst.begin(); i != last; i++)
    {
        dns_naptr *rr = *i;
        bool res;

        if( strlen(rr->replacement) )
        {
#ifdef DEBUG_OUTPUT
            cerr << "MATCH len " << strlen(rr->replacement) << " " << "'" << rr->replacement << "'" << endl;
#endif
            result_type = NONE;
            result = rr->replacement;
            res = true;
        }
        else{
            std::string regexp = rr->regexp;

            res = calc_regexp( regexp );
        }

        if( res )
        {
            switch (toupper(rr->flags[0]))
            {
            case 'S':
                result_type = SRV;
                break;
            case 'A':
                result_type = ADDR;
                break;
            case 'U':
                result_type = URI;
                break;
            }

            service = rr->service;

            transform( service.begin(), service.end(),
                       service.begin(), (int(*)(int))toupper );

#ifdef DEBUG_OUTPUT
            dump_naptr_entry(rr);
#endif
            return true;
        }
    }

    return false;
}

bool Dns_Naptr_Query_Priv::dump_naptr( dns_rr_naptr *naptr )
{
    list<dns_naptr *> lst;

    for (int i=0; i < naptr->dnsnaptr_nrr; i++)
    {
        bool handle = false;
        dns_naptr *rr = &naptr->dnsnaptr_naptr[i];
#ifdef DEBUG_OUTPUT
        dump_naptr_entry(rr);
#endif

        if (strlen(rr->flags) > 1)
            continue;

        switch (rr->flags[0])
        {
        case 'S':
        case 's':
        case 'U':
        case 'u':
            //       dns_rr_srv *srv = (dns_rr_srv*)
            // 	dns_resolve_p(ctx, rr->replacement, DNS_C_IN, DNS_T_SRV, DNS_NOSRCH,
            // 		      dns_parse_srv);

            //       dump_srv(srv);
            handle = true;
            break;
        default:
#ifdef DEBUG_OUTPUT
            cerr << "Unhandled" << endl;
#else
            ;
#endif
        }

        std::string service = rr->service;

        transform( service.begin(), service.end(),
                   service.begin(), (int(*)(int))toupper );

        list<std::string>::const_iterator iter =
                find(accept_services->begin(), accept_services->end(),
                     service);

        if( iter == accept_services->end() )
        {
#ifdef DEBUG_OUTPUT
            cerr << "Unhandled " << rr->service << endl;
#endif
            handle = false;
        }


        if( handle )
            lst.push_back(rr);
    }

    lst.sort(dns_naptr_pred);

    bool res = process( lst );

#ifdef DEBUG_OUTPUT
    cerr << endl << "DUMP" << endl;
    dump_naptr_list(lst);
#endif

    return res;
}

bool Dns_Naptr_Query_Priv::resolve( const std::string &init_domain,
                                    const std::string &theTarget )
{
    std::string domain = init_domain;

    target = theTarget;

    for( int max_depth = 10 ; max_depth > 0; max_depth-- )
    {

        dns_rr_naptr *naptr = NULL;

        naptr = dns_resolve_naptr(ctx, domain.c_str(), DNS_NOSRCH);

        if (!naptr)
        {
            return false;
        }

#ifdef DEBUG_OUTPUT
        cerr << "#records " << naptr->dnsnaptr_nrr << endl;
#endif

        bool res = dump_naptr(naptr);

        free(naptr);

        if( !res )
            return false;

        if ( result_type != 0 )
        {
#ifdef DEBUG_OUTPUT
            cerr << "resolve " << get_result() << endl;
#endif
            return true;
        }

        domain = result;
    }
    return false;
}


