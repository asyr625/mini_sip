/*
 * PlayerScoreData.h
 *
 *  Created on: Apr 21, 2014
 *      Author: vcafe
 */

#include <string>

#ifndef PLAYERSCOREDATA_H_
#define PLAYERSCOREDATA_H_




typedef struct _PlayerScoreData
{
	std::string name;
	std::string date;
	std::string time;
	unsigned int score;

} PlayerScoreData;


#endif /* PLAYERSCOREDATA_H_ */
