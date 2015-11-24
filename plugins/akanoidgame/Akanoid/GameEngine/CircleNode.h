/*
 * CircleNode.h
 *
 *  Created on: Apr 17, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#ifndef CIRCLENODE_H_
#define CIRCLENODE_H_

#include "Node.h"
#include "ColorRGBA.hpp"

namespace GameEngine {

class CircleNode: public GameEngine::Node {
	float _radius;
	ColorRGBA _color;

public:
	CircleNode();
	CircleNode(float r);
	virtual ~CircleNode();

	void setRadius(float r);
	float getRadius();
//	void setColor(ColorRGBA &color);
	void setColor(ColorRGBA color);

	void draw();
};

} /* namespace GameEngine */

#endif /* CIRCLENODE_H_ */
