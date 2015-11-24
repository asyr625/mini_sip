/*
 * Item.cpp
 *
 *  Created on: May 28, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include "Item.h"

namespace GameEngine {

Item::Item()
{

	srand(time(NULL));
	int item = rand() % 18;

	//printf("\nITEM %d\n",item);

	if( item <= 1) // Chance 10 %
	{	_itemAbility = LIFE;
		_itemLabel.setText("L");
		_shape.setColor(ColorRGBA(220,0,0,150));
	}
	else if( item >= 2 && item <= 4) // Chance 20%
	{
		_itemAbility = MULTIPLE_BALL;
		_itemLabel.setText("M");
		_shape.setColor(ColorRGBA(220,100,50,150));
	}
	else if( item > 4 && item <= 7) // Chance 20+ %
	{
		_itemAbility = SUPER_BALL;
		_itemLabel.setText("S");
		_shape.setColor(ColorRGBA(200,220,0,150));
	}
	else if ( item > 7 && item <= 12)
	{
		_itemAbility = LONG_PADDLE;
		_itemLabel.setText("P");
		_shape.setColor(ColorRGBA(200,0,200,150));
	}
	else
	{
		_itemAbility = BONUS_SCORE;
		_itemLabel.setText("B");
		_shape.setColor(ColorRGBA(0,200,200,150));
	}

	_itemLabel.setFontSize(20);
	_itemLabel.setColor(ColorRGBA(250,250,250));


	_isItemAlive = true;
	_shape.setSize(itemHeight, itemHeight);
	_shape.setOrigin(itemHeight / 2.f, itemHeight / 2.f);
}

Item::~Item()
{

}

int Item::getItemProperty()
{
	return _itemAbility;
}

bool Item::isAlive()
{
	return _isItemAlive;
}
void Item::destroy()
{
	_isItemAlive = false;
}

void Item::setRenderWindow(OpenGlWindow* window)
{
	_itemLabel.setRenderWindow(window);
}

void Item::draw()
{
	if(_isItemAlive == false)
		return;
	_shape.draw();
	_itemLabel.draw();
}

void Item::update(double elapsed)
{
	if(_isItemAlive == false)
		return;

	float py = itemVelocity * elapsed;
	float my = y() + py;
	float mx = x();

	if(my > gScreenHeight || my < 0)
	{
		this->destroy();
		return;
	}
	_shape.setPosition(mx,my);
	_itemLabel.setPosition(mx-9,my-10);
}

void Item::setPosition(float x, float y)
{
	_shape.setPosition(x,y);
	_itemLabel.setPosition(x,y);
}

float Item::x() { return _shape.getPosition().x; }
float Item::y() { return _shape.getPosition().y; }
float Item::left() { return x() - _shape.getSize().x / 2.f; }
float Item::right() { return x() + _shape.getSize().x / 2.f; }
float Item::top() { return y() - _shape.getSize().y / 2.f; }
float Item::bottom() { return y() + _shape.getSize().y / 2.f; }



} /* namespace GameEngine */
