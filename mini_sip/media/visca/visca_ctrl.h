#ifndef MVISCACTRL_H
#define MVISCACTRL_H

#include<string>
#include "sobject.h"
#include "timeout_provider.h"

struct Lib_Visca_Internal;

class Visca_Ctrl
{
public:
    Visca_Ctrl(std::string device);
    ~Visca_Ctrl();

	/**
	* @param pspeed pan speed, -128..127. Negative=pan left
	* @param tspeed tilt speed, -128..127. Negative=pan down 
	* @param zspeed zoom speed, -128..127. Negative=zoom wide
	*/
    void set_pan_tilt_zoom(int pspeed, int tspeed, int zspeed, int durationMs);

	void timeout(int);

private:
    static int map_speed(int charVal, int imin, int imax);

	bool initialized;

    SRef<Timeout_Provider<int, Visca_Ctrl*> * > timeoutProvider;
    Lib_Visca_Internal* internal;

	int ps,pd; //pan speed (-128..127, negative is left) and duration (ms)
	int ts,td;
	int zs,zd;
};

#endif

