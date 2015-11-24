#ifndef SIP_UTILS_H
#define SIP_UTILS_H
#include <string>
class Sip_Utils
{
public:
    static bool starts_with(std::string line, std::string part);

    static int find_end_of_header(const char *buf, unsigned bufSize,  int &startIndex);
    static int find_end_of_header(const std::string &buf, int &startIndex);
};

#endif // SIP_UTILS_H
