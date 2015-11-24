/*
 * ScoreBoard.h
 *
 *  Created on: Apr 23, 2014
  *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include "./GameEngine/RectangleNode.h"
#include "./GameEngine/TextNode.h"
#include "./Util/PlayerScoreData.hpp"
#include <minisip/OpenGlWindow.h>

#ifndef SCOREBOARD_H_
#define SCOREBOARD_H_

namespace GameEngine {

class LeaderBoard:public GameEngine::Node {

	RectangleNode _background;
	RectangleNode _line;
	RectangleNode _lineBelow;

	std::string _playerName;
	std::vector<TextNode> _leaderTextNode;
	std::vector<TextNode> _localTextNode;

	TextNode _touchToNextLabel;
	TextNode _leaderTitleLabel;
	TextNode _localTitleLabel;
	TextNode _latestScoreLabel;

	OpenGlWindow* _renderWindow;

	int _latestScore;

	void ensureRenderWindow();

public:

	void setPosition(float x, float y);
	void setSize(float width, float height);

	LeaderBoard();
	void setTitle();
	virtual ~LeaderBoard();

	void setPlayerName(std::string name);
	void setRenderWindow(OpenGlWindow* window);

	void setScore(int score);
	void draw();

	void setHighScore(std::vector<PlayerScoreData> data);
	void setLocalScore(std::vector<PlayerScoreData> data);

};

} /* namespace GameEngine */

#endif /* SCOREBOARD_H_ */
