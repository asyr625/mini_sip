/*
 * Block.cpp
 *
 *  Created on: May 14, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include "Block.h"

namespace GameEngine {


Block::Block()
{
	_blockLevel = 0;
	_isBlockAlive = true;
	_shape.setSize(blockWidth, blockHeight);
	_shape.setOrigin(blockWidth / 2.f, blockHeight / 2.f);
	_shape.setColor(ColorRGBA(200,0,0,150));
}

Block::Block(int x, int y)
{
	_blockLevel = 0;
	_isBlockAlive = true;
	_shape.setPosition(x, y);
	_shape.setSize(blockWidth, blockHeight);
	_shape.setOrigin(blockWidth / 2.f, blockHeight / 2.f);
	_shape.setColor(ColorRGBA(200,0,0,150));
}


void Block::updateLevel()
{
	switch(_blockLevel)
	{
		case 0: _shape.setColor(ColorRGBA(200,0,0,150)); break;
		case 1: _shape.setColor(ColorRGBA(0,200,0,150)); break;
		case 2: _shape.setColor(ColorRGBA(0,0,200,150)); break;
		case 3: _shape.setColor(ColorRGBA(200,200,0,150)); break;
	}

}

bool Block::increseLevel()
{
	_blockLevel++;
	if(_blockLevel > 3)
		return false;

	updateLevel();

	return true;
}

bool Block::isAlive()
{
	return _isBlockAlive;
}

void Block::destroy()
{
	_blockLevel--;
	if( _blockLevel < 0)
		_isBlockAlive = false;
	else
		updateLevel();

}

void Block::draw()
{
	_shape.draw();
}

void Block::setPosition(float x, float y)
{
	_shape.setPosition(x,y);
}

float Block::x() { return _shape.getPosition().x; }
float Block::y() { return _shape.getPosition().y; }
float Block::left() { return x() - _shape.getSize().x / 2.f; }
float Block::right() { return x() + _shape.getSize().x / 2.f; }
float Block::top() { return y() - _shape.getSize().y / 2.f; }
float Block::bottom() { return y() + _shape.getSize().y / 2.f; }


} /* namespace GameEngine */
