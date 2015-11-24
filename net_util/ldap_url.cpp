#include "ldap_url.h"
#include "ldap_exception.h"
#include "string_utils.h"

#ifdef ENABLE_LDAP

#ifdef WIN32
#include <windows.h>
#include <winldap.h>
#else
#include <ldap.h>
#endif

#endif

Ldap_Url::Ldap_Url(std::string url)
{
#ifdef ENABLE_LDAP
    clear();
    set_uri(url);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

Ldap_Url::Ldap_Url()
{
    clear();
}

void Ldap_Url::clear()
{
#ifdef ENABLE_LDAP
    host = "";
    port = LDAP_PORT;
    filter = "(objectClass=*)";
    dn = "";
    scope = LDAP_SCOPE_BASE;
    valid_url = false;
    attributes = std::vector<std::string>();
    extensions = std::vector<Ldap_Url_Extension>();
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

bool Ldap_Url::is_valid() const
{
    return valid_url;
}

std::string Ldap_Url::get_string() const
{
#ifdef ENABLE_LDAP
    // Start off with the schema and host name
    std::string url("ldap://");
    url += host;
    if (port > 0) {
        url += ':';
        url += itoa(port);
    }

    // Append distinguished name (base DN)
    url += '/';
    url += percent_encode(dn, false, true);

    // Append attributes
    url += '?';
    if (attributes.size() > 0) {
        for (size_t i=0; i<attributes.size(); i++) {
            if (i>0)
                url += ',';
            url += percent_encode(attributes.at(i), false, true);
        }
    }

    // Append scope
    url += '?';
    url += (scope == LDAP_SCOPE_BASE ? "base" : (scope == LDAP_SCOPE_SUBTREE ? "sub" : "one"));

    // Append filter
    url += '?';
    if (filter.length() > 0)
    {
        url += filter;
    }

    // Append extensions
    if (extensions.size() > 0)
    {
        url += '?';
        for (size_t i=0; i<extensions.size(); i++)
        {
            if (i>0)
                url += ',';

            Ldap_Url_Extension ext = extensions.at(i);
            if (ext.critical)
                url += '!';
            url += percent_encode(ext.type, false, true);
            url += '=';
            url += percent_encode(ext.value, true, true);
        }
    }

    return url;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

bool Ldap_Url::is_unreserved_char(char in) const
{
    const char* alphabetUnreserved = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";
    for (int i=0; i<66; i++)
    {
        if (alphabetUnreserved[i] == in)
            return true;
    }
    return false;
}

bool Ldap_Url::is_reserved_char(char in) const
{
    const char* alphabetReserved = ":/?#[]@!$&'()*+,;=";
    for (int i=0; i<18; i++)
    {
        if (alphabetReserved[i] == in)
            return true;
    }
    return false;
}

void Ldap_Url::set_url(const std::string url)
{
#ifdef ENABLE_LDAP
    std::string::size_type lastPos = 0, pos = 0, posTemp = 0;

    if (str_case_cmp(url.substr(0, 7).c_str(), "ldap://") == 0)
    {
        lastPos = 7;

        /**************************************************************
         * Search for host and port
         */

        pos = url.find('/', lastPos);

        if (pos == std::string::npos)
        {
            // No slash after schema specifier. This means that the URL at most specified a host and a port.
            pos = url.length();
        }

        if (pos != lastPos)
        {
            // Host or port found
            posTemp = url.find(':', lastPos);
            if (posTemp != std::string::npos && posTemp < pos)
            {
                // Port found
                host = url.substr(lastPos, posTemp - lastPos);
                port = atoi(url.substr(posTemp+1).c_str());
            }
            else
            {
                host = url.substr(lastPos, pos - lastPos);
            }
        }
        lastPos = pos+1;

        if (lastPos < url.length())
        {
            std::string restOfUrl = url.substr(lastPos);

            std::vector<std::string> parts = split(restOfUrl, false, '?', true);

            switch (parts.size())
            {
            case 5:
            {
                std::vector<std::string> exts = split(parts.at(4), false, ',', true);
                for (size_t i=0; i<exts.size(); i++)
                {
                    std::string ext = exts.at(i);
                    std::string::size_type colonPos = ext.find('=', 0);
                    bool critical = (ext[0] == '!');
                    int criticalOffset = (critical ? 1 : 0);
                    if (colonPos != std::string::npos) {
                        extensions.push_back(Ldap_Url_Extension(percent_decode(ext.substr(criticalOffset,colonPos-criticalOffset)),percent_decode(ext.substr(colonPos+1)), critical));
                    } else {
                        extensions.push_back(Ldap_Url_Extension(percent_decode(ext.substr(criticalOffset)),"", critical));
                    }
                }
            }
            case 4:
                filter = parts.at(3);
            case 3:
                if (0 == str_case_cmp(parts.at(2).c_str(), "one")) {
                    scope = LDAP_SCOPE_ONELEVEL;
                } else if (0 == str_case_cmp(parts.at(2).c_str(), "base")) {
                    scope = LDAP_SCOPE_BASE;
                } else if (0 == str_case_cmp(parts.at(2).c_str(), "sub")) {
                    scope = LDAP_SCOPE_SUBTREE;
                } else {
                    valid_url = false;
                }

            case 2:
                attributes = split(parts.at(1), false, ',', true);
                for (size_t i=0; i<attributes.size(); i++) {
                    attributes.at(i) = percent_decode(attributes.at(i));
                }
            case 1:
                dn = percent_decode(parts.at(0));
            }
        }
        valid_url = true;
    } else {
        valid_url = false;
    }
#endif
}

void Ldap_Url::print_debug()
{
#ifdef ENABLE_LDAP
    std::cerr <<  "     VALID?      " << (valid_url ? "yes" : "NO") << std::endl;

    std::cerr <<  "     Host:       [" << host << "]" << std::endl;

    std::cerr <<  "     Port:       [" << port << "]" << std::endl;

    std::cerr <<  "     Attributes: " << std::endl;
    for(size_t i = 0; i < attributes.size(); i++)
        std::cerr <<  "                 [" << attributes.at(i) << "]" << std::endl;

    std::cerr <<  "     Extensions: " << std::endl;
    for (size_t i = 0; i < extensions.size(); i++)
        std::cerr <<  "                 [" << extensions.at(i).type << "=" << extensions.at(i).value << "]" << (extensions.at(i).critical ? " (critical!)" : "") << std::endl;

    std::cerr <<  "     Filter:     [" << filter << "]" << std::endl;

    std::cerr <<  "     DN:         [" << dn << "]" << std::endl;

    std::cerr <<  "     Scope:      [" << (scope == LDAP_SCOPE_BASE ? "base" : (scope == LDAP_SCOPE_ONELEVEL ? "one" : "sub")) << "]" << std::endl;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

bool Ldap_Url::has_critical_extension() const
{
    for (size_t i=0; i<extensions.size(); i++)
        if (extensions.at(i).critical)
            return true;
    return false;
}

std::string Ldap_Url::get_host() const
{
    return host;
}
void Ldap_Url::set_host(std::string host_)
{
    host = host_;
}

int32_t Ldap_Url::get_port() const
{
    return port;
}

void Ldap_Url::set_port(int32_t p)
{
    port = p;
}

std::vector<std::string> Ldap_Url::get_attributes() const
{
    return attributes;
}

void Ldap_Url::set_attributes(std::vector<std::string> attr)
{
    attributes = attr;
}

std::vector<Ldap_Url_Extension> Ldap_Url::get_extensions() const
{
    return extensions;
}


std::string Ldap_Url::get_filter() const
{
    return filter;
}

void Ldap_Url::set_filter(std::string filter_)
{
    this->filter = filter_;
}

std::string Ldap_Url::get_dn() const
{
    return dn;
}

void Ldap_Url::set_dn(std::string dn_)
{
    dn = dn_;
}

int32_t Ldap_Url::get_scope() const
{
    return scope;
}

void Ldap_Url::set_scope(int32_t s)
{
    scope = s;
}

std::string Ldap_Url::encode_char(const char in) const
{
    std::string res;
    res += '%';
    if (in < 10)
        res +='0';
    res += bin_to_hex(reinterpret_cast<const unsigned char*>(&in), sizeof(in));
    return res;
}

char Ldap_Url::decode_char(const std::string in) const
{
    if (in.length() == 3)
    {
        return (char_to_num(in[1]) << 4) + (char_to_num(in[2]));
    }
    else
    {
        return '0';
    }
}
int32_t Ldap_Url::char_to_num(const char in) const
{
    if (in >= '0' && in <= '9')
    {
        return (in - '0');
    }
    else if (in >= 'A' && in <= 'F')
    {
        return (in - 'A' + 10);
    }
    else if (in >= 'a' && in <= 'f')
    {
        return (in - 'a' + 10);
    }
    else
    {
        return -1;
    }
}

std::string Ldap_Url::percent_decode(const std::string & in) const
{
    std::string res;
    for(size_t i = 0; i < in.length(); i++)
    {
        if ('%' == in[i])
        {
            res += decode_char(in.substr(i, 3));
            i+=2;
        } else
            res += in[i];
    }
    return res;
}

std::string Ldap_Url::percent_encode(const std::string & in, bool escapeComma, bool escapeQuestionmark ) const
{
    std::string res;
    for(size_t i = 0; i < in.length(); i++)
    {
        if( (!is_reserved_char(in[i]) && !is_unreserved_char(in[i])) ||
                (escapeQuestionmark && in[i] == '?') ||
                (escapeComma && in[i] == ',') )
            res += encode_char(in[i]);
        else
            res += in[i];
    }
    return res;
}
