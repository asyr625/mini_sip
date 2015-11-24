/*
 * TextNode.h
 *
 *  Created on: Apr 23, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include <string>
#include <minisip/OpenGlWindow.h>
#include "ColorRGBA.hpp"
#include "Node.h"

#ifndef TEXTNODE_H_
#define TEXTNODE_H_

namespace GameEngine{



template<typename T>
std::string numberToString(T number)
{
	std::ostringstream ss;
	ss << number;
	return ss.str();
}


class TextNode : public GameEngine::Node  {
	std::string _text;
	int _fontSize;
	OpenGlWindow *_renderWindow;
	ColorRGBA _color;

public:
	void setText(std::string t);
	void setColor(ColorRGBA color);
	void draw();
	void setFontSize(int size);
	void setRenderWindow(OpenGlWindow* window);
	TextNode();
	virtual ~TextNode();
};

} /* namespace GameEngine */

#endif /* TEXTNODE_H_ */
