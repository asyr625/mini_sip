/*
 * TextNode.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */
#include <GL/gl.h>
#include "TextNode.h"

#define SCALE 1.4

namespace GameEngine {

TextNode::TextNode() {
	_fontSize = 14;
	_renderWindow = 0;

}

TextNode::~TextNode() {
}


void TextNode::setRenderWindow(OpenGlWindow *window)
{
	_renderWindow = window;
}

void TextNode::setColor(ColorRGBA color)
{
	_color.setColor(color);
}

void TextNode::setText(std::string t)
{
	_text.assign(t);
}

void TextNode::setFontSize(int size)
{
	_fontSize = size;
}


void TextNode::draw()
{
	if(this->isVisible() == false)
		return;

	if( _renderWindow == NULL )
		return;

//	float x = getPosition().x;
//	float y = getPosition().y;

	unsigned char r = _color.r() * 255;
	unsigned char g = _color.b() * 255;
	unsigned char b = _color.b() * 255;


	SDL_Color foregroundColor = { r, g, b };
	SDL_Color backgroundColor = { 0, 0, 0 };


//	SDL_Color foregroundColor = { 150, 205, 155 };
//	SDL_Color backgroundColor = { 255, 255, 255 };

	if (_fontSize <=0)
		return;


	MRef<TextTexture*> t =  _renderWindow->getText()->getTextureObject(_text, _fontSize, foregroundColor, backgroundColor);
	//glEnable2D();
	massert(glGetError()==GL_NO_ERROR);
	glDisable(GL_DEPTH_TEST);

	//glBlendFunc(GL_ONE, GL_ONE);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	float txWidth = t->getTextureWidth();
	float txHeight = t->getTextureHeight();

	this->setSize(txWidth,txHeight);

	glEnable(GL_TEXTURE_2D);

	int tex = t->getTexture();

	glColor4fv(_color);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex );

	glBindTexture(GL_TEXTURE_2D, tex );
	glBegin(GL_QUADS);


	float x1 = getPosition().x - getOrigin().x ;
	float y1 = getPosition().y - getOrigin().y;

	float x2 = x1 + txWidth ;
	float y2 = y1 + txHeight ;

//	glVertex2f (x1,y1);
//	glVertex2f (x1,y2);
//	glVertex2f (x2,y2);
//	glVertex2f (x2,y1);

//	glTexCoord2f(0, 0); glVertex3f(x, y, 0);
//	glTexCoord2f(0, 1); glVertex3f(x, y + txHeight, 0);
//	glTexCoord2f(1, 1); glVertex3f(x + txWidth, y + txHeight, 0);
//	glTexCoord2f(1, 0); glVertex3f(x + txWidth, y, 0);


	glTexCoord2f(0, 0); glVertex2f (x1,y1);
	glTexCoord2f(0, 1); glVertex2f (x1,y2);
	glTexCoord2f(1, 1); glVertex2f (x2,y2);
	glTexCoord2f(1, 0); glVertex2f (x2,y1);


	glEnd();

	glDisable(GL_TEXTURE_2D);
}


} /* namespace GameEngine */
