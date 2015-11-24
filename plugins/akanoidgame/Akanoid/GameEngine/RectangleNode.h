/*
 * RectangleNode.h
 *
 *  Created on: Apr 17, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#ifndef RECTANGLENODE_H_
#define RECTANGLENODE_H_

#include "Node.h"
#include "ColorRGBA.hpp"

namespace GameEngine {

class RectangleNode: public GameEngine::Node {
private:
	ColorRGBA _color;

public:
	RectangleNode();
	virtual ~RectangleNode();
	void draw();
	void setColor(ColorRGBA color);
};

} /* namespace GameEngine */

#endif /* RECTANGLENODE_H_ */
