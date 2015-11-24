/*
 * Ball.cpp
 *
 *  Created on: May 14, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */
#include <SDL/SDL.h>
#include "Ball.h"

namespace GameEngine {

Ball::~Ball()
{
}


Ball::Ball()
: velocity(-ballVelocity,ballVelocity)
{

	_isVisible = true;
	_shape.setPosition(0,0);
	_shape.setRadius(ballRadius);
	_shape.setColor(ColorRGBA(45,226,180,140));


	_innerShape.setPosition(0,0);
	_innerShape.setRadius(ballRadius - 3);
	_innerShape.setColor(ColorRGBA(255,255,255,140));

	_isBallDead = false;
	_effectDuration = 0;
	_effectTime = 0;

	_level = 0;
}

void Ball::setLevel(int lv)
{
	_level = lv;
}

bool Ball::isDead()
{
	return _isBallDead;
}

void Ball::setVisible(bool visible)
{
	_isVisible = visible;
}

void Ball::start()
{
	_isBallDead = false;
	velocity.x = -ballVelocity;
	velocity.y = ballVelocity;
}


void Ball::stop()
{
	velocity.x = 0;
	velocity.y = 0;
}

void Ball::move(float dx , float dy)
{
	float mx = x() + dx;
	float my = y() + dy;


	if(mx > gScreenWidth || mx < 0)
		return;

	if(my > gScreenHeight || my < 0)
		return;

	_shape.setPosition(mx,my);
	_innerShape.setPosition(mx,my);
}

void Ball::setPosition(int x, int y)
{
	_shape.setPosition(x,y);
	_innerShape.setPosition(x,y);
}

void Ball::update(double elapsed)
{

	if(_isBallDead == true)
		return;

	float dx = velocity.x * elapsed;
	float dy = velocity.y * elapsed;

	move(dx,dy);

	//float factor = level * 0.01;
	float levelFactor = _level * 0.012;
	float velo = (ballVelocity + levelFactor);


	if(left() < 0)
		velocity.x = velo;

	// Towards the right, set horizontal velocity to
	// a negative value (towards the left).
	else if(right() > gScreenWidth )
		velocity.x = -velo;

	// Top/bottom collisions.
	if(top() < 0) velocity.y = velo;
	else if(bottom() > gScreenHeight)
	{
		_isBallDead = true;
		velocity.y = -velo;
	}


	//if ball has double size effect
	if( _effectDuration > 0)
	{
		unsigned int milSec = SDL_GetTicks() - _effectTime;
		unsigned int durationLeft = _effectDuration - milSec;

		if( durationLeft <= 4000 )
		{
			// warning user with blink Animation
			if( durationLeft % 1000 > 500)
				_shape.setColor(ColorRGBA(25,116,160,240));
			else
				_shape.setColor(ColorRGBA(45,226,180,140));


			// time less than 2 sec.  Animate
			if( durationLeft <= 2000 && durationLeft >= 0)
			{
				double width = _shape.getSize().x - 0.001;
				if(width > ballRadius )
					this->setWidth(width);
				else
					this->setWidth(ballRadius);
			}
			// no time left , reset width.
			if( durationLeft <= 0 )
			{
				_effectDuration = 0;
				this->setWidth(ballRadius);
			}
		}

	}

}

void Ball::setWidth(float r)
{
	_shape.setRadius(r);
	_innerShape.setRadius(r - 3);
	_shape.setColor(ColorRGBA(45,226,180,140));
}

void Ball::doubleSize(float duration)
{
	this->setWidth(ballRadius * 3);

	_effectDuration = (duration * 1000);
	_effectTime = SDL_GetTicks();

}


void Ball::draw()
{
	if( _isVisible == false || _isBallDead == true)
		return;

	_innerShape.draw();
	_shape.draw();
}


float Ball::x() { return _shape.getPosition().x; }
float Ball::y() { return _shape.getPosition().y; }
float Ball::left() { return x() - _shape.getRadius(); }
float Ball::right() { return x() + _shape.getRadius(); }
float Ball::top() { return y() - _shape.getRadius(); }
float Ball::bottom() { return y() + _shape.getRadius(); }


} /* namespace GameEngine */

