/*
 * ColorRGBA.h
 *
 *  Created on: Apr 17, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#ifndef COLORRGBA_H_
#define COLORRGBA_H_

namespace GameEngine {

class ColorRGBA {
private:
	GLfloat _color[4];
public:
	ColorRGBA(){ setColor(0,0,0,0);}
	ColorRGBA(int r, int g , int b , int a = 255)
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
	virtual ~ColorRGBA(){}

	void setColor(ColorRGBA& color)
	{
		_color[0] = color.r();
		_color[1] = color.g();
		_color[2] = color.b();
		_color[3] = color.a();
	}


	void setColor (int r, int g , int b , int a = 255)
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
};

} /* namespace GameEngine */

#endif /* COLORRGBA_H_ */
