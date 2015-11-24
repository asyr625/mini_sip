#ifndef COLOR_RGBA_H
#define COLOR_RGBA_H


namespace Game_Engine
{

class Color_RGBA
{
public:
    Color_RGBA() { set_color(0,0,0,0);}
    Color_RGBA(int r, int g , int b , int a = 255)
    {
        float rf = r/255.0f;
        float gf = g/255.0f;
        float bf = b/255.0f;
        float af = a/255.0f;

        _color[0] = rf;
        _color[1] = gf;
        _color[2] = bf;
        _color[3] = af;
    }
    virtual ~Color_RGBA(){}

    void set_color(Color_RGBA& color)
    {
        _color[0] = color.r();
        _color[1] = color.g();
        _color[2] = color.b();
        _color[3] = color.a();
    }


    void set_color (int r, int g , int b , int a = 255)
    {
        _color[0] = (float)r/255.0;
        _color[1] = (float)g/255.0;
        _color[2] = (float)b/255.0;
        _color[3] = (float)a/255.0;
    }

//	void setColor(float r, float g , float b , float a = 1.0)
//	{
//		_color[0] = r;
//		_color[1] = g;
//		_color[2] = b;
//		_color[3] = a;
//	}

    operator const GLfloat* ()const
    {
        return (float*)_color;
    }

    float r(){return _color[0];}
    float g(){return _color[1];}
    float b(){return _color[2];}
    float a(){return _color[3];}
private:
    GLfloat _color[4];
};

}
#endif // COLOR_RGBA_H
