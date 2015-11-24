#include "file_url.h"
#include "string_utils.h"
#include <iostream>
using namespace std;
File_Url::File_Url()
{
    clear();
}

File_Url::File_Url(const std::string url)
{
    clear();
    set_url(url);
}

File_Url::File_Url(const std::string url, const int32_t t)
    : type(t)
{
    clear();
    set_url(url);
}

void File_Url::clear()
{
    host = "";
    type = FILEURL_TYPE_UNKNOWN;
    path = "";
    valid_url = false;
}

bool File_Url::is_valid() const
{
    return valid_url;
}

std::string File_Url::get_string() const
{
    std::string url("file://");
    url += host;

    // Append distinguished name (base DN)
    url += '/';

    // Split path into parts using "\" in Windows and "/" on other system
    char sep = (type == FILEURL_TYPE_WINDOWS ? '\\' : '/');
    std::vector<std::string> parts = split(path, false, sep, true);

    // Glue together each "path part" again
    for (size_t i=0; i<parts.size(); i++) {
        std::string decPart = percent_encode(parts.at(i));
        url += decPart + '/';
    }

    // Strip away the final trailing "/"
    url = url.substr(0, url.length() - 1);
    return url;
}

bool File_Url::is_unreserved_char(char in) const
{
    const char* alphabetUnreserved = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";
    for (int i=0; i<66; i++)
    {
        if( alphabetUnreserved[i] == in )
            return true;
    }
    return false;
}

bool File_Url::is_reserved_char(char in) const
{
    const char* alphabetReserved = ":/?#[]@!$&'()*+,;=";
    for (int i=0; i<18; i++)
    {
        if( alphabetReserved[i] == in )
            return true;
    }
    return false;
}

void File_Url::set_url(const std::string url)
{
    std::string::size_type lastPos = 0, pos = 0;

    if (str_case_cmp(url.substr(0, 7).c_str(), "file://") == 0) {
        lastPos = 7;

        /**************************************************************
        * Search for host
        */
        pos = url.find('/', lastPos);

        if (pos == std::string::npos)
        {
            // No slash after schema specifier. This means that no path has been specified. Illegal.
            valid_url = false;
            return;
        }

        if (pos != lastPos)
        {
            // Host found
            host = url.substr(lastPos, pos - lastPos);
        }
        lastPos = pos+1;

        if (lastPos < url.length())
        {
            std::string restOfUrl = url.substr(lastPos);

            // Split the "path part of the URL" into pieces, each piece separated by "/" as specified by RFC 1738.
            std::vector<std::string> parts = split(restOfUrl, false, '/', true);

            // Glue the pieces together using an operating-system specific separator
            for (size_t i=0; i<parts.size(); i++)
            {
                std::string decPart = percent_decode(parts.at(i));
                if (type == FILEURL_TYPE_WINDOWS)
                {
                    path += decPart + '\\';
                } else {
                    path += decPart + '/';
                }
            }
            if (path.length() > 1)
            {
                // Remove trailing "path separator character"
                path = path.substr(0, path.length() - 1);
            } else {
                valid_url = false;
            }
        }
        valid_url = true;
    }
    else
    {
        valid_url = false;
    }
}

void set_url(const std::string url, const int32_t type)
{

}

void File_Url::print_debug()
{
    std::cerr <<  "     VALID?      " << (valid_url ? "yes" : "NO") << std::endl;

    std::cerr <<  "     Host:       [" << host << "]" << std::endl;

    std::cerr <<  "     Path:       [" << path << "]" << std::endl;

    std::cerr <<  "     Type:       [";
    switch (type)
    {
    case FILEURL_TYPE_UNIX:
        std::cerr << " UNIX";
        break;
    case FILEURL_TYPE_WINDOWS:
        std::cerr << " Windows";
        break;
    default:
        std::cerr << " Unspecified";
        break;
    }
    std::cerr <<  "]" << std::endl;
}

std::string File_Url::get_host() const
{
    return host;
}
void File_Url::set_host(const std::string h)
{
    host = h;
}

std::string File_Url::get_path() const
{
    return path;
}

void File_Url::set_path(const std::string p)
{
    path = p;
}

int32_t File_Url::get_type() const
{
    return type;
}

void File_Url::set_type(const int32_t t)
{
    type = t;
}

std::string File_Url::encode_char(const char in) const
{
    std::string res;
    res += '%';
    if (in < 10)
        res +='0';
    res += bin_to_hex(reinterpret_cast<const unsigned char*>(&in), sizeof(in));
    return res;
}
char File_Url::decode_char(const std::string in) const
{
    if (in.length() == 3) {
        return (char_to_num(in[1]) << 4) + (char_to_num(in[2]));
    } else {
        return '0';
    }
}
int32_t File_Url::char_to_num(const char in) const
{
    if (in >= '0' && in <= '9') {
        return (in - '0');
    } else if (in >= 'A' && in <= 'F') {
        return (in - 'A' + 10);
    } else if (in >= 'a' && in <= 'f') {
        return (in - 'a' + 10);
    } else {
        return -1;
    }
}
std::string File_Url::percent_decode(const std::string & in) const
{
    std::string res;
    for (size_t i=0; i < in.length(); i++) {
        if ('%' == in[i]) {
            res += decode_char(in.substr(i, 3));
            i+=2;
        } else
            res += in[i];
    }
    return res;
}

std::string File_Url::percent_encode(const std::string & in) const
{
    return percent_encode(in, true, true);
}

std::string File_Url::percent_encode(const std::string & in, bool escapeComma, bool escapeQuestionmark) const
{
    std::string res;
    for (size_t i=0; i < in.length(); i++)
    {
        if ((!is_reserved_char(in[i]) && !is_unreserved_char(in[i])) ||
                (escapeQuestionmark && in[i] == '?') || (escapeComma && in[i] == ',') )
            res += encode_char(in[i]);
        else
            res += in[i];
    }
    return res;
}
