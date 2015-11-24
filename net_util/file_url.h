#ifndef FILE_URL_H
#define FILE_URL_H

#include "my_types.h"
#include <vector>
#include <string>

#define FILEURL_TYPE_UNKNOWN 	0
#define FILEURL_TYPE_UNIX 	1
#define FILEURL_TYPE_WINDOWS 	2

class File_Url
{
public:
    File_Url(const std::string url);
    File_Url(const std::string url, const int32_t t);
    File_Url();

    void clear();

    bool is_valid() const;

    void set_url(const std::string url);
    void set_url(const std::string url, const int32_t type);

    std::string get_string() const;

    void print_debug();

    std::string get_host() const;
    void set_host(const std::string h);

    std::string get_path() const;
    void set_path(const std::string p);

    int32_t get_type() const;
    void set_type(const int32_t t);
private:
    bool is_unreserved_char(char in) const;
    bool is_reserved_char(char in) const;
    std::string encode_char(const char in) const;
    char decode_char(const std::string in) const;
    int32_t char_to_num(const char in) const;
    std::string percent_decode(const std::string & in) const;
    std::string percent_encode(const std::string & in) const;
    std::string percent_encode(const std::string & in, bool escapeComma, bool escapeQuestionmark = true) const;

    int32_t     type;
    std::string host;
    std::string path;
    bool valid_url;
};

#endif // FILE_URL_H
