/*
 * NextLVBoard.cpp
 *
 *  Created on: May 9, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include "ScoreBoard.h"

namespace GameEngine {

ScoreBoard::ScoreBoard()
{
	_renderWindow = NULL;
	_titleLabel.setText("Level 1 completed");
	_latestScoreLabel.setText("Total score: 0");
}

ScoreBoard::~ScoreBoard()
{

}

void ScoreBoard::setRenderWindow(OpenGlWindow* window)
{
	_renderWindow = window;
	_titleLabel.setRenderWindow(_renderWindow);
	_latestScoreLabel.setRenderWindow(_renderWindow);
}

void ScoreBoard::setScoreLevel(int score, int level)
{
	std::string scoreText = "Total score : ";
	scoreText += numberToString(score);

	std::string levelText = "Level ";
	levelText += numberToString(level);
	levelText += " completed";

	_titleLabel.setText(levelText);
	_latestScoreLabel.setText(scoreText);

}

void ScoreBoard::setPosition(float x, float y)
{

	_background.setColor(ColorRGBA(0,0,0,180));
	_background.setPosition(x,y);

	int px = (x - (_background.getSize().x/2.0)) + 20;

	_titleLabel.setPosition(px,y+20);
	_latestScoreLabel.setPosition(px,y+60);

}


void ScoreBoard::setSize(float width, float height)
{

	_background.setOrigin(width/2.0 ,0);
	_background.setSize(width,height);

	_titleLabel.setFontSize(30);
	_titleLabel.setColor(ColorRGBA(250,250,250));

	_latestScoreLabel.setFontSize(20);
	_latestScoreLabel.setColor(ColorRGBA(250,250,250));

	if( _renderWindow != NULL)
	{
		_titleLabel.setRenderWindow(_renderWindow);
		_latestScoreLabel.setRenderWindow(_renderWindow);
	}

}

void ScoreBoard::draw()
{
	if(this->isVisible() == false)
		return;

	_background.draw();
	_titleLabel.draw();
	_latestScoreLabel.draw();
}



} /* namespace GameEngine */
