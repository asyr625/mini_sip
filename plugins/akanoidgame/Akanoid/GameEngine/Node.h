/*
 * Node.h
 *
 *  Created on: Apr 15, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */



#include <GL/gl.h>


#ifndef NODE_H_
#define NODE_H_


#include <iostream>
#include <vector>
//#include <algorithm>


extern int gScreenWidth;
extern int gScreenHeight;


namespace GameEngine
{

typedef struct _Vector2
{
	float x;
	float y;

	_Vector2()
	{
		x = 0;
		y = 0;
	}
	_Vector2(float px, float py)
	{
		x = px;
		y = py;
	}

} Vector2;

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;

class Node {
private:
	std::string	 		_name;
	Uint32				_id;
	Node*               _parent;
	std::vector<Node*>	_children;
	Uint8				_zValue;
	Vector2				_size;
	bool				_visible;
	bool				_enabled;

	Vector2			_position;
	Vector2			_origin;


public:
	Node();
	Node(std::string name);
	virtual ~Node();

    const Uint32    	getId() const;
    void                setName(const std::string& name);
    const std::string&  getName() const;

    void                setParent(Node* parent);
    Node*               getParent() const;

    void                setSize(const Vector2& size);
    void                setSize(float width, float height);
    const Vector2& 		getSize() const;


    void                setPosition(const Vector2& position);
    void                setPosition(float x, float y);
    const Vector2& 		getPosition() const;


    void                setOrigin(const Vector2& origin);
    void                setOrigin(float x, float y);
    const Vector2& 		getOrigin() const;


    void                add(Node* object);
    void                remove(Node* object);
    void                clear();

    std::vector<Node*>& getChildren();
    Node*               findObjectByName(const std::string& name);

    Vector2        		getAbsolutePosition() const;

    void                setZValue(Uint8 zvalue);
    Uint8           	getZValue() const;

    void                setVisible(bool visible);
    bool                isVisible() const;

    void                setEnabled(bool enabled);
    bool                isEnabled() const;

    virtual void        onResize() {}
    virtual void        onKeyDown(int key);
    virtual void        update(float elapsedTime);
    bool                isHit(int x , int y);



    static Uint32   _nextId;


};

}
#endif /* NODE_H_ */


