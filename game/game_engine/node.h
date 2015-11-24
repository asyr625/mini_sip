#ifndef NODE_H
#define NODE_H

#include <GL/gl.h>

#include <iostream>
#include <vector>


extern int gscreen_width;
extern int gscreen_height;

namespace Game_Engine
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

class Node
{
public:
    Node();
    Node(std::string name);
    virtual ~Node();

    const Uint32    	get_id() const;
    void                set_name(const std::string& name);
    const std::string&  get_name() const;

    void                set_parent(Node* parent);
    Node*               get_parent() const;

    void                set_size(const Vector2& size);
    void                set_size(float width, float height);
    const Vector2& 		get_size() const;


    void                set_position(const Vector2& position);
    void                set_position(float x, float y);
    const Vector2& 		get_position() const;


    void                set_origin(const Vector2& origin);
    void                set_origin(float x, float y);
    const Vector2& 		get_origin() const;


    void                add(Node* object);
    void                remove(Node* object);
    void                clear();

    std::vector<Node*>& get_children();
    Node*               find_object_by_name(const std::string& name);

    Vector2        		get_absolute_position() const;

    void                set_zvalue(Uint8 zvalue);
    Uint8           	get_zvalue() const;

    void                set_visible(bool visible);
    bool                is_visible() const;

    void                set_enabled(bool enabled);
    bool                is_enabled() const;

    virtual void        on_resize() {}
    virtual void        on_key_down(int key);
    virtual void        update(float elapsedTime);
    bool                is_hit(int x , int y);

    static Uint32   _nextId;

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
};

}
#endif // NODE_H
