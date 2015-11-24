/*
 * Paddle.h
 *
 *  Created on: May 14, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */
#include "GameEngine/RectangleNode.h"

#ifndef PADDLE_H_
#define PADDLE_H_

namespace GameEngine {

const float paddleVelocity = 0.32;
const float paddleWidth = 100.0f;
const float paddleHeight = 20.0f;

class Paddle {
private:
	bool _isVisible;
	RectangleNode _shape;
	RectangleNode _innerShape;

	Vector2 _velocity;
	unsigned int _effectTime;
	unsigned int _effectDuration;

	void setWidth(float width);

public:
	Paddle();
	virtual ~Paddle(){};

	void doubleSize(float duration);
	void reset();

	void setVisible(bool visible);
	void setPosition(float x, float y);
	void move(float px);
	void update(double elapsed);
	void draw();
	float x();
	float y();
	float left();
	float right();
	float top();
	float bottom();


};

} /* namespace GameEngine */

#endif /* PADDLE_H_ */
