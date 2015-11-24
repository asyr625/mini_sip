/*
 * RectangleNode.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include "RectangleNode.h"

namespace GameEngine {

RectangleNode::RectangleNode() {
	// TODO Auto-generated constructor stub

}

RectangleNode::~RectangleNode() {
	// TODO Auto-generated destructor stub
}

void RectangleNode::setColor(ColorRGBA color)
{
	_color.setColor(color);
}

void RectangleNode::draw()
{
	float x1 = getPosition().x - getOrigin().x ;
	float y1 = getPosition().y - getOrigin().y;

	float x2 = x1 + getSize().x ;
	float y2 = y1 + getSize().y ;

	glBegin(GL_TRIANGLE_FAN);
		glColor4fv(_color);
		glVertex2f (x1,y1);
		glVertex2f (x1,y2);
		glVertex2f (x2,y2);
		glVertex2f (x2,y1);
	glEnd();

}

} /* namespace GameEngine */
