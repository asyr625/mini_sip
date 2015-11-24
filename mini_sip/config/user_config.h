#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <string>

class User_Config
{
public:
    static std::string get_mini_sip_home_directory(void);
    static std::string get_file_name(std::string baseName);

protected:
    User_Config() {}
};

#endif // USER_CONFIG_H
