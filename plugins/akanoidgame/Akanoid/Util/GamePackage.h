//
//  GamePackage.h
//  Serial
//
//  Created by Ter on 4/4/14.
//  Copyright (c) 2014 Objective-C Book. All rights reserved.
//

#ifndef __Serial__GamePackage__
#define __Serial__GamePackage__

#include <iostream>
#include <cstring>

class GamePackage {

private:
    unsigned char readIndex;
    unsigned char writeIndex;
    char buffer[256];

public:
    GamePackage();
    ~GamePackage();

    void writeByte(char data);
    void writeShort(short int data);
    void writeInt(unsigned int data);
    void writeFloat(float data);
    void writeString(std::string data);

    char readByte();
    short int readShort();
    unsigned int readInt();
    float readFloat();
    std::string readString();

    char* networkData();
    void setNetworkData(char* data , int len);
    int size();
    void reset();

};
#endif /* defined(__Serial__GamePackage__) */


