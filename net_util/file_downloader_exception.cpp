#include "file_downloader_exception.h"


File_Downloader_Exception::File_Downloader_Exception()
{
    msg = "File_Downloader_Exception";
}

File_Downloader_Exception::File_Downloader_Exception(std::string msg)
{
    this->msg = "File_Downloader_Exception: " + msg;
}

const char* File_Downloader_Exception::what()const throw()
{
    return msg.c_str();
}

File_Downloader_Not_Found_Exception::File_Downloader_Not_Found_Exception(std::string message)
{
    msg = "File_Downloader_Not_Found_Exception: Could not find file " + message;
}
