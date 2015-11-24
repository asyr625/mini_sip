#ifndef VIDEO_EXCEPTION_H
#define VIDEO_EXCEPTION_H

#include <string>

class Video_Exception
{
public:
    Video_Exception( std::string message ):message(message){}
    std::string error() { return message; }
private:
    std::string message;
};

#endif // VIDEO_EXCEPTION_H
