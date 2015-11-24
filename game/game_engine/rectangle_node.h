#ifndef RECTANGLE_NODE_H
#define RECTANGLE_NODE_H

#include "node.h"
#include "color_rgba.h"

namespace Game_Engine
{

class Rectangle_Node: public Game_Engine::Node
{
public:
    Rectangle_Node();
    virtual ~Rectangle_Node();
    void draw();
    void set_color(ColorRGBA color);
private:
    Color_RGBA _color;

};

}

#endif // RECTANGLE_NODE_H
