#ifndef FILE_DOWNLOADER_H
#define FILE_DOWNLOADER_H

#include "file_url.h"
#include "downloader.h"
#include "file_downloader_exception.h"

#include <cstring>
#include <vector>
#include <map>

class File_Downloader : public Downloader
{
public:
    File_Downloader(std::string originalUrl);

    char* get_chars(int *length);

    void save_to_file(std::string filename) throw (File_Downloader_Exception);
    std::string get_string() throw (File_Downloader_Exception);


    virtual std::string get_mem_object_type() const { return "FileDownloader"; }
private:
    bool fetch(std::ostream & outStream);
    File_Url url;
};

#endif // FILE_DOWNLOADER_H
