/*
 * NextLVBoard.h
 *
 *  Created on: May 9, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include "./GameEngine/RectangleNode.h"
#include "./GameEngine/TextNode.h"
#include <minisip/OpenGlWindow.h>


#ifndef NEXTLVBOARD_H_
#define NEXTLVBOARD_H_

namespace GameEngine {

class ScoreBoard : public GameEngine::Node {

	TextNode _titleLabel;
	TextNode _latestScoreLabel;
	RectangleNode _background;

	OpenGlWindow* _renderWindow;

public:
	void setRenderWindow(OpenGlWindow* window);
	void setScoreLevel(int score, int level);
	void setSize(float width, float height);
	void setPosition(float x, float y);

	void draw();

	ScoreBoard();
	virtual ~ScoreBoard();
};

} /* namespace GameEngine */

#endif /* NEXTLVBOARD_H_ */
