#ifndef FILE_SYSTEM_UTILS_H
#define FILE_SYSTEM_UTILS_H

#include <list>

#include "sobject.h"

class File_System_Utils : public SObject
{
public:
    static std::list<std::string> directory_contents(std::string dir, bool includeSubdirectories);
private:
    static void directory_contents_internal(std::string dir, bool includeSubdirectories, std::list<std::string> & res);
};

#endif // FILE_SYSTEM_UTILS_H
