/*
 * Paddle.cpp
 *
 *  Created on: May 14, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <SDL/SDL.h>

#include "Paddle.h"

namespace GameEngine {


Paddle::Paddle()
{
	reset();
};

void Paddle::setVisible(bool visible)
{
	_isVisible = visible;
}

void Paddle::setWidth(float width)
{
	_shape.setSize(width,paddleHeight);
	_shape.setOrigin(width/2.0 ,paddleHeight/2.0 );

	_innerShape.setSize(width - 4,paddleHeight -4 );
	_innerShape.setOrigin((width - 4)/2.0 ,(paddleHeight - 4)/2.0 );
	_innerShape.setColor(ColorRGBA(70,26,18,200));

}

void Paddle::doubleSize(float duration)
{
	this->setWidth(paddleWidth * 2);
	_effectDuration = (duration * 1000);
	_effectTime = SDL_GetTicks();
}

void Paddle::reset()
{
	_effectDuration = 0;
	_effectTime = 0;

	_shape.setColor(ColorRGBA(245,226,180,200));
	_shape.setSize(paddleWidth,paddleHeight );
	_shape.setOrigin(paddleWidth/2.0 ,paddleHeight/2.0 );


	_innerShape.setColor(ColorRGBA(70,26,18,200));
	_innerShape.setSize(paddleWidth - 4,paddleHeight -4 );
	_innerShape.setOrigin((paddleWidth - 4)/2.0 ,(paddleHeight - 4)/2.0 );

	_isVisible =true;

}

void Paddle::move(float px)
{

	float mx = x() + px;
	float my = y();

	if(mx > gScreenWidth || mx < 0)
		return;

	_shape.setPosition(mx,my);
	_innerShape.setPosition(mx,my);
}

void Paddle::setPosition(float x, float y)
{
	_shape.setPosition(x,y);
	_innerShape.setPosition(x,y);
}


void Paddle::update(double elapsed)
{

	if( _effectDuration > 0)
	{
		unsigned int milSec = SDL_GetTicks() - _effectTime;
		unsigned int durationLeft = _effectDuration - milSec;

		if( durationLeft <= 5000 )
		{
			// warning user with blink Animation
			if( durationLeft % 1000 > 500)
				_innerShape.setColor(ColorRGBA(70,26,18,50));
			else
				_innerShape.setColor(ColorRGBA(70,26,18,200));

			// time less than 1 sec.  Animate
			if( durationLeft <= 1000 && durationLeft > 0)
			{
				double width = _shape.getSize().x - 10;
				if(width > paddleWidth )
					this->setWidth(width);
				else
					this->setWidth(paddleWidth);
			}
			// no time left , reset width.
			if( durationLeft <= 0 )
			{
				_effectDuration = 0;
				this->setWidth(paddleWidth);
			}
		}

	}
}

void Paddle::draw()
{
	if( _isVisible == false)
		return;

	_shape.draw();
	_innerShape.draw();
}



float Paddle::x() { return _shape.getPosition().x; }
float Paddle::y() { return _shape.getPosition().y; }
float Paddle::left() { return    x() - _shape.getSize().x / 2; }
float Paddle::right() { return x() + _shape.getSize().x / 2; }
float Paddle::top() { return y() - _shape.getSize().y / 2; }
float Paddle::bottom() { return y() + _shape.getSize().y / 2; }


} /* namespace GameEngine */
