/*
 * GameRtcpReport.h
 *
 *  Created on: Apr 8, 2014
 *      Author: vcafe
 */



#ifndef GAMERTCPREPORT_H_
#define GAMERTCPREPORT_H_

#include <libminisip/media/rtp/RtcpReport.h>
#include "GamePackage.h"


class GameRtcpReport : public RtcpReport
{
private:
	GamePackage* gamePackage;
	bool done;

public:
	GameRtcpReport();
	GameRtcpReport(GamePackage &data);
	GameRtcpReport(void * buf, int max_length);
	virtual ~GameRtcpReport();

	GamePackage* getGamePackage();

	int getLength();
	void writeData(char* buf);

	virtual void debugPrint();
	bool isDone();

	//unsigned int getSenderSSRC();
};


#endif /* GAMERTCPREPORT_H_ */
