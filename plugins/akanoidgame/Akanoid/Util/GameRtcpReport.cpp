/*
 * GameRtcpReport.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: vcafe
 */

#include <iostream>
#include "GameRtcpReport.h"
#define HEADER_OFFSET 4

GameRtcpReport::~GameRtcpReport()
{
	if( gamePackage != NULL)
		delete(gamePackage);
	gamePackage = NULL;

	//std::cout << "Game rtcp clear" << std::endl;
}

GameRtcpReport::GameRtcpReport()	: RtcpReport(204)
{
	done = false;
	gamePackage = NULL;
	gamePackage = new GamePackage();
}

GameRtcpReport::GameRtcpReport(void * buf, int max_length)
	: RtcpReport(204)
{
	done = false;
	gamePackage = new GamePackage();
	gamePackage->setNetworkData((char*)buf,max_length);
}

GameRtcpReport::GameRtcpReport(GamePackage &data)
	: RtcpReport(204)
{
	done = false;
	gamePackage = new GamePackage();
	gamePackage->setNetworkData((char*)data.networkData(),data.size());
}

void GameRtcpReport::writeData(char* buf)
{
	done = true;
	writeHeader(buf);
	//int s = gamePackage->size();
	std::cout << "\nsending .." << gamePackage->networkData() << std::endl;
	memcpy(buf,gamePackage->networkData(),gamePackage->size() );

}

bool GameRtcpReport::isDone()
{
	return done;
}

int GameRtcpReport::getLength()
{
	return gamePackage->size() + HEADER_OFFSET;
}

GamePackage* GameRtcpReport::getGamePackage()
{
	return gamePackage;
}

void GameRtcpReport::debugPrint()
{

}
