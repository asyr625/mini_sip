#ifndef TEXT_NODE_H
#define TEXT_NODE_H

#include <string>
#include <minisip/OpenGlWindow.h>
#include "color_rgba.h"
#include "node.h"

namespace Game_Engine
{

template<typename T>
std::string number_to_string(T number)
{
    std::ostringstream ss;
    ss << number;
    return ss.str();
}


class Text_Node : public Game_Engine::Node
{
public:
    Text_Node();
    virtual ~Text_Node();
    void set_text(std::string t);
    void set_color(ColorRGBA color);
    void draw();
    void set_font_size(int size);
    void set_render_window(OpenGlWindow* window);

private:
    std::string _text;
    int _font_size;
    OpenGlWindow *_renderWindow;
    Color_RGBA _color;
};

}
#endif // TEXT_NODE_H
