/*
 * CircleNode.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */
#include <GL/gl.h>
#include <cmath>
#include <stdio.h>
#include "CircleNode.h"

#define SEGMENT 48

namespace GameEngine {

CircleNode::CircleNode() {
	// TODO Auto-generated constructor stub
	_radius = 4;
}

CircleNode::~CircleNode() {
	// TODO Auto-generated destructor stub
}


void CircleNode::setRadius(float r)
{
	_radius = r;
}
float CircleNode::getRadius()
{
	return _radius;
}
void CircleNode::setColor(ColorRGBA color)
{
	_color.setColor(color);
}
//void CircleNode::setColor(ColorRGBA &color)
//{
//	_color.setColor(color);
//}

void CircleNode::draw()
{

	GLfloat twicePi = 2.0 * M_PI;
	float x = getPosition().x;
	float y = getPosition().y;

	glBegin(GL_TRIANGLE_FAN);
		glColor4fv(_color);
		glVertex2f(x, y);
		for (int n = 0; n <= SEGMENT ; ++n)
		{
			glVertex2f (

					x + (_radius * std::cos ( n* twicePi / SEGMENT)) ,
					y + (_radius * std::sin ( n* twicePi / SEGMENT))
			);
		}
	glEnd();

}

} /* namespace GameEngine */
