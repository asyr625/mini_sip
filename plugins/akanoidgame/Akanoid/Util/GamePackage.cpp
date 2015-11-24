//
//  GamePackage.cpp
//  Serial
//
//  Created by Ter on 4/4/14.
//  Copyright (c) 2014 Objective-C Book. All rights reserved.
//
#include <arpa/inet.h>
#include "GamePackage.h"
#define MAX_SIZE 256

GamePackage::GamePackage()
{
	writeIndex = 0;
	readIndex = 0;
    memset(buffer, 0, 256);
}

GamePackage::~GamePackage() {
}


void GamePackage::writeByte(char data)
{
    memcpy(&buffer[writeIndex], &data, sizeof(char));
    writeIndex++;
}

void GamePackage::writeShort(short int data)
{
	short int hostData = htons(data);
    memcpy(&buffer[writeIndex], &hostData, sizeof(short int));
    writeIndex+= sizeof(short int);
}

void GamePackage::writeInt(unsigned int data)
{

	unsigned int hostData = htonl(data);
    memcpy(&buffer[writeIndex], &hostData, sizeof(int));
    writeIndex+= sizeof(int);
}

void GamePackage::writeFloat(float data)
{
    memcpy(&buffer[writeIndex], &data, sizeof(float));
    writeIndex+= sizeof(float);
}

void GamePackage::writeString(std::string data)
{
    writeByte(data.length());
    memcpy(&buffer[writeIndex], data.c_str(), data.length());
    writeIndex+= data.length();
}

char GamePackage::readByte()
{
    char result;
    memcpy(&result, &buffer[readIndex], sizeof(char));
    readIndex+= sizeof(char);
    return result;
}

short int GamePackage::readShort()
{
    short int result;
    short int raw;
    memcpy(&raw, &buffer[readIndex], sizeof(short int));
    result = ntohs(raw);
    readIndex+= sizeof(short int);
    return result;
}

unsigned int GamePackage::readInt()
{
	unsigned int raw;
    memcpy(&raw, &buffer[readIndex], sizeof(unsigned int));

    unsigned int result = ntohl(raw);

    readIndex+= sizeof(unsigned int);
    return result;
}

float GamePackage::readFloat()
{
    float result;
    memcpy(&result, &buffer[readIndex], sizeof(float));
    readIndex+= sizeof(float);
    return result;
}

std::string GamePackage::readString()
{
    char len = readByte();
    char temp[256] = {0};

    memcpy(&temp, &buffer[readIndex], len);
    readIndex+= len;

    std::string result(temp);
    return result;
}

char* GamePackage::networkData()
{
    return buffer;
}

void GamePackage::setNetworkData(char* data, int len)
{
    reset();
    if(len < MAX_SIZE)
    	memcpy(buffer, data, len);
    else
    	memcpy(buffer,data, MAX_SIZE);

    readIndex = 0;
    writeIndex = len;
}

int GamePackage::size()
{
    return writeIndex;
}

void GamePackage::reset()
{
    memset(buffer, 0, 256);
    writeIndex = 0;
    readIndex = 0;
}
