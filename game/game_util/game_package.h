#ifndef GAME_PACKAGE_H
#define GAME_PACKAGE_H

#include <iostream>
#include <cstring>

class Game_Package
{
public:
    Game_Package();
    ~Game_Package();

    void write_byte(char data);
    void write_short(short int data);
    void write_int(unsigned int data);
    void write_float(float data);
    void write_string(std::string data);

    char read_byte();
    short int read_short();
    unsigned int read_int();
    float read_float();
    std::string read_string();

    char* network_data();
    void set_network_data(char* data , int len);
    int size();
    void reset();

private:
    unsigned char read_index;
    unsigned char write_index;
    char buffer[256];
};

#endif // GAME_PACKAGE_H
