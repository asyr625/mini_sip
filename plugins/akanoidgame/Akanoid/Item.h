/*
 * Item.h
 *
 *  Created on: May 28, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */


#include <minisip/OpenGlWindow.h>
#include "GameEngine/RectangleNode.h"
#include "GameEngine/TextNode.h"
#include <stdlib.h>
#include <ctime>

#ifndef ITEM_H_
#define ITEM_H_

namespace GameEngine {


const float itemWidth = 20.0f;
const float itemHeight = 20.0f;
const float itemVelocity = 0.12;


enum ITEM
{
	NONE,
	LONG_PADDLE,
	MULTIPLE_BALL,
	BONUS_SCORE,
	LIFE,
	SUPER_BALL
};

class Item {
private:
	int _itemAbility;
	bool _isItemAlive;
	RectangleNode _shape;
	TextNode _itemLabel;

	void move(float py);

public:
	Item();
	virtual ~Item();

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
	void update(double elapsed);
	int getItemProperty();
	void setRenderWindow(OpenGlWindow* window);


};

} /* namespace GameEngine */

#endif /* ITEM_H_ */
