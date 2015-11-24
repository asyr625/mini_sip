/*
 * Node.cpp
 *
 *  Created on: Apr 15, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */
#include <algorithm>
#include "Node.h"

int gScreenWidth = 0;
int gScreenHeight = 0;

namespace GameEngine
{


Uint32  Node::_nextId = 0;

Node::Node(std::string name) :
		_name(name),
		_parent(0),
		_zValue(0),
		_visible(true),
		_enabled(true)
{
	_id = _nextId;
	_nextId++;

}

Node::Node() :
		_name(""),
		_parent(0),
		_zValue(0),
		_visible(true),
		_enabled(true)
{
	_id = _nextId;
	_nextId++;
}

Node::~Node() {
}


void Node::setName(const std::string& name)
{
    _name = name;
}

const std::string&   Node::getName() const
{
    return _name;
}

const Uint32    Node::getId() const
{
    return _id;
}

void    Node::setZValue(Uint8 zvalue)
{
    _zValue = zvalue;
}

Uint8   Node::getZValue() const
{
    return _zValue;
}

void    Node::setVisible(bool visible)
{
    _visible = visible;
}

bool    Node::isVisible() const
{
    return _visible;
}

void    Node::setEnabled(bool enabled)
{
   _enabled = enabled;
}

bool    Node::isEnabled() const
{
    return _enabled;
}


void    Node::setParent(Node* parent)
{
    _parent = parent;
}

Node* Node::getParent() const
{
    return _parent;
}



void    Node::add(Node* object)
{
    if (!object)
        return;

    std::vector<Node*>::iterator it ;
    for (it = _children.begin(); it != _children.end(); ++it)
        if (*it == object)
            return;

    _children.push_back(object);
    object->setParent(this);

}

void    Node::remove(Node* object)
{
    if (!object)
        return;

    std::vector<Node*>::iterator it ;

    for (it = _children.begin(); it != _children.end(); ++it)
    {
    	if (*it == object)
        {
            object->setParent(0);
            _children.erase(it);
            return;
        }
    }
}

void    Node::clear()
{
	_children.clear();
}


std::vector<Node*>& Node::getChildren()
{
    return _children;
}

void    Node::setSize(const Vector2& size)
{
    _size = size;
    onResize();
}

void Node::setSize(float width, float height)
{
    setSize(Vector2(width, height));
}

const Vector2& Node::getSize() const
{
    return _size;
}

void Node::setPosition(const Vector2& position)
{
	_position = position;
}
void Node::setPosition(float x, float y)
{
	setPosition(Vector2(x,y));
}

const Vector2& Node::getPosition() const
{
	return _position;
}


void Node::setOrigin(const Vector2& origin)
{
	_origin = origin;
}
void Node::setOrigin(float x, float y)
{
	setOrigin(Vector2(x,y));
}

const Vector2& Node::getOrigin() const
{
	return _origin;
}


//
//void                Node::setGLPosition(const Vector2f& glPosition)
//{
//	_glPosition = glPosition;
//}
//
//void                Node::setGLPosition(float x, float y)
//{
//	setGLPosition(x,y);
//}
//
//const Vector2f& 	Node::getGLPosition() const
//{
//	return _glPosition;
//}


Vector2 Node::getAbsolutePosition() const
{
    Vector2 absPos = getPosition();
    Node* p = getParent();

    while (p)
    {
        const Vector2& pos = p->getPosition();
        absPos.x += pos.x;
        absPos.y += pos.y;

        p = p->getParent();
    }
    return absPos;
}
//
//void    Node::onReceiveEvent(const sf::Event& event)
//{
//    if (!isEnabled())
//        return;
//}


static bool    OrderByZOrder(Node* obj1, Node* obj2)
{
    return obj1->getZValue() < obj2->getZValue();
}

void    Node::update(float elapsedTime)
{
    // on trie d'abord notre liste d'objets
    std::sort(_children.begin(), _children.end(), &OrderByZOrder);

    size_t max = _children.size();
    for (size_t i = 0; i < max; ++i)
    {
    	Node* child = _children[i];

        if (child)
            if (child->isEnabled())
                child->update(elapsedTime);
    }
}


void Node::onKeyDown(int key)
{
	if(_enabled ==false)
		return;
}

//void Node::draw(sf::RenderTarget &target, sf::RenderStates states) const
//{
//
//    if (!isVisible())
//        return;
//
//    size_t max = _children.size();
//    for (size_t i = 0; i < max; ++i)
//    {
//    	Node* child = _children[i];
//
//        if (child->isVisible())
//        {
//        	states.transform *= getTransform();
//        	target.draw(*child,states);
//        }
//    }
//
//}

Node* Node::findObjectByName(const std::string& name)
{
    size_t max = _children.size();
    for (size_t it = 0; it != max; ++it)
    {
        Node* child = _children[it];

        if (child->getName() == name)
            return child;
    }
    return 0;
}

bool Node::isHit(int x , int y)
{

    if (x > getPosition().x && x < ( getPosition().x + getSize().x))
    {
        if( y> getPosition().y && y< (getPosition().y + getSize().y))
            return true;
    }

    return false;
}

}

