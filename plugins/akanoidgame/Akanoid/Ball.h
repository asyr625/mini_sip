/*
 * Ball.h
 *
 *  Created on: May 14, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include "GameEngine/CircleNode.h"
#include "GameEngine/Node.h"

#ifndef BALL_H_
#define BALL_H_

namespace GameEngine {

const float ballVelocity = 0.18f;
const float ballRadius = 12.0f;

class Ball {
private:

	CircleNode _shape;
	CircleNode _innerShape;

	bool _isVisible;
	bool _isBallDead;

	unsigned int _effectTime;
	unsigned int _effectDuration;

	int _level;

	void setWidth(float width);

public:
	Vector2 velocity;

	Ball();
	virtual ~Ball();

	void doubleSize(float duration);
	void setLevel(int lv);


	void start();
	void stop();

	bool isDead();
	void setVisible(bool visible);
	void move(float dx , float dy);
	void setPosition(int x, int y);
	void update(double elapsed);
	void draw();

	float x() ;
	float y();
	float left();
	float right();
	float top();
	float bottom();
};

} /* namespace GameEngine */

#endif /* BALL_H_ */
