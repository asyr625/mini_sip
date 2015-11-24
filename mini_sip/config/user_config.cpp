#include "user_config.h"
#include "dbg.h"

#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

std::string User_Config::get_mini_sip_home_directory(void)
{
    struct stat st;
    std::string MiniSIPHomeDirectoryName = string(getenv("HOME")) + "/.minisip";

    if (stat(MiniSIPHomeDirectoryName.c_str(), &st) == 0)
    {
        if (!S_ISDIR(st.st_mode))
        {
            cerr << "[UserConfig] It is a file and not directory" << endl;
            unlink(MiniSIPHomeDirectoryName.c_str());
            if (mkdir(MiniSIPHomeDirectoryName.c_str(), 0777) == -1)
            {
                cerr << "[UserConfig] Error opening the new directory" << endl;
            }
        }
    }
    else
    {
        if (mkdir(MiniSIPHomeDirectoryName.c_str(), 0777) == -1)
        {
            cerr << "[UserConfig] Error creating the new directory" << endl;
        } else
            cerr << "Directory created successfully" << endl;
    }
    return MiniSIPHomeDirectoryName;
}

std::string User_Config::get_file_name(std::string baseName)
{
    return (get_mini_sip_home_directory() + string("/") + baseName);
}
