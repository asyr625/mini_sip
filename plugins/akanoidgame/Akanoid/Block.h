/*
 * Block.h
 *
 *  Created on: May 14, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */


#include "GameEngine/RectangleNode.h"

#ifndef BLOCK_H_
#define BLOCK_H_

namespace GameEngine {

const float blockWidth = 82.0f;
const float blockHeight = 24.0f;


class Block
{
	int _blockLevel;
	bool _isBlockAlive;
	RectangleNode _shape;

public:

	Block();
	Block(int x, int y);
	virtual ~Block(){}

	void doubleSize(float duration);

	void updateLevel();
	bool increseLevel();
	bool isAlive();
	void destroy();
	void draw();
	void setPosition(float x, float y);
	float x();
	float y();
	float left();
	float right();
	float top();
	float bottom();
};

} /* namespace GameEngine */

#endif /* BLOCK_H_ */
