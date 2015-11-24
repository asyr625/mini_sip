/*
 * LeaderBoard.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>

#include "LeaderBoard.h"

namespace GameEngine {


LeaderBoard::LeaderBoard() {
	_renderWindow = NULL;
	_latestScore = 0;
}

LeaderBoard::~LeaderBoard() {

}

void LeaderBoard::setRenderWindow(OpenGlWindow* window)
{
	_renderWindow = window;
	_leaderTitleLabel.setRenderWindow(_renderWindow);
	_latestScoreLabel.setRenderWindow(_renderWindow);
}


void LeaderBoard::setScore(int score)
{
	std::string text = "Your latest score is ";
	text += numberToString(score);
	_latestScoreLabel.setText( text);
}

void LeaderBoard::setSize(float width, float height)
{
	//float y = getPosition().y;

	_background.setOrigin(width/2.0 ,0);
	_background.setSize(width,height);

	_line.setOrigin(width/2.0 ,0);
	_line.setSize(width - 20 , 2);
	_line.setColor(ColorRGBA(240,180,30));

	_lineBelow.setOrigin(width/2.0 ,0);
	_lineBelow.setSize(width - 20 , 2);
	_lineBelow.setColor(ColorRGBA(240,180,30));


	_leaderTitleLabel.setPosition(500, 16);
	_leaderTitleLabel.setFontSize(20);
	_leaderTitleLabel.setColor(ColorRGBA(250,250,250));

	_localTitleLabel.setPosition(454, 164);
	_localTitleLabel.setFontSize(20);
	_localTitleLabel.setColor(ColorRGBA(250,250,250));


	_touchToNextLabel.setPosition(414,height - 10);
	_touchToNextLabel.setColor(ColorRGBA(250,250,250));
	_touchToNextLabel.setFontSize(14);
	_touchToNextLabel.setText("*Touch anywhere to start the game");

	_latestScoreLabel.setPosition(180, height - 60);
	_latestScoreLabel.setFontSize(20);
	_latestScoreLabel.setColor(ColorRGBA(250,250,250));
	_latestScoreLabel.setText("Your latest score is 0");

}


void LeaderBoard::setPosition(float x, float y)
{
	Node::setPosition(x,y);

	_background.setColor(ColorRGBA(0,0,0,180));
	_background.setPosition(x,y);

	float centerx = (_background.getSize().x - _line.getSize().x ) / 2.0;
	_line.setPosition(x + centerx,y + 32);
	_lineBelow.setPosition(x + centerx,y + 182 );

	_leaderTitleLabel.setText("Leaderboard");
	_localTitleLabel.setText("Your highest scores");

}

void LeaderBoard::ensureRenderWindow()
{
	_localTitleLabel.setRenderWindow(_renderWindow);
	_leaderTitleLabel.setRenderWindow(_renderWindow);
	_latestScoreLabel.setRenderWindow(_renderWindow);
	_touchToNextLabel.setRenderWindow(_renderWindow);

	int size = _leaderTextNode.size();
	if( size == 0)
		return;

	for(int i = 0 ; i < size ; i++)
	{
		_leaderTextNode[i].setRenderWindow(_renderWindow);
	}

	size = _localTextNode.size();
	for(int i = 0 ; i < size ; i++)
	{
		_localTextNode[i].setRenderWindow(_renderWindow);
	}

}

void LeaderBoard::draw()
{

	if(this->isVisible() == false)
		return;

	_background.draw();
	_line.draw();
	_lineBelow.draw();
	_leaderTitleLabel.draw();
	_localTitleLabel.draw();

	_latestScoreLabel.draw();
	_touchToNextLabel.draw();

	// leader
	int size = _leaderTextNode.size();
	if( size != 0)
	{
		for(int i = 0 ; i < size ; i++)
		_leaderTextNode[i].draw();
	}


	// local
	size = _localTextNode.size();
	if( size != 0)
	{
		for(int i = 0 ; i < size ; i++)
			_localTextNode[i].draw();
	}



}

void LeaderBoard::setPlayerName(std::string name)
{
	_playerName.clear();
	_playerName.assign(name);
}

void LeaderBoard::setLocalScore(std::vector<PlayerScoreData> data)
{
	_localTextNode.clear();
	int size = data.size();
    for( int i = 0 ; i < size; i++)
    {
    	ColorRGBA white = ColorRGBA(255,255,255);
    	ColorRGBA red = ColorRGBA(240,180,80);

    	TextNode dateNode;
    	dateNode.setFontSize(20);
    	dateNode.setText(data[i].date);
    	dateNode.setPosition(180,180 + (26*(i+1)));


    	TextNode nameNode;
    	nameNode.setFontSize(20);
    	nameNode.setText(data[i].name);
    	nameNode.setPosition(340,180 + (26*(i+1)));


    	TextNode scoreNode;
    	scoreNode.setFontSize(20);
    	scoreNode.setText(numberToString(data[i].score));
    	scoreNode.setPosition(560,180 + (26*(i+1)));

        dateNode.setColor(white);
        nameNode.setColor(white);
        scoreNode.setColor(white);

        _localTextNode.push_back(dateNode);
        _localTextNode.push_back(nameNode);
        _localTextNode.push_back(scoreNode);


    }

    ensureRenderWindow();

}



void LeaderBoard::setHighScore(std::vector<PlayerScoreData> data)
{
	_leaderTextNode.clear();
	int size = data.size();
    for( int i = 0 ; i < size; i++)
    {
    	ColorRGBA white = ColorRGBA(255,255,255);
    	ColorRGBA red = ColorRGBA(240,180,80);

    	TextNode dateNode;
    	dateNode.setFontSize(20);
    	dateNode.setText(data[i].date);
    	dateNode.setPosition(180,40 + (26*(i+1)));



    	TextNode nameNode;
    	nameNode.setFontSize(20);
    	nameNode.setText(data[i].name);
    	nameNode.setPosition(340,40 + (26*(i+1)));


    	TextNode scoreNode;
    	scoreNode.setFontSize(20);
    	scoreNode.setText(numberToString(data[i].score));
    	scoreNode.setPosition(560,40 + (26*(i+1)));

    	// set color
    	if(_playerName.compare(data[i].name) == 0)
    	{
        	dateNode.setColor(white);
        	nameNode.setColor(white);
        	scoreNode.setColor(white);
    	}
    	else
    	{
        	dateNode.setColor(red);
        	nameNode.setColor(red);
        	scoreNode.setColor(red);

    	}

    	_leaderTextNode.push_back(dateNode);
    	_leaderTextNode.push_back(nameNode);
    	_leaderTextNode.push_back(scoreNode);


    }

    ensureRenderWindow();
}




} /* namespace GameEngine */
