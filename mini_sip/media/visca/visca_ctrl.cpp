
#include "visca_ctrl.h"

#ifdef VISCA_CAMCTRL
#include<libvisca/libvisca.h>
#endif

#include<iostream>
#include<stdio.h>

using namespace std;

#ifdef VISCA_CAMCTRL
struct Lib_Visca_Internal
{
	VISCAInterface_t interface;
        VISCACamera_t camera;
        unsigned char packet[3000];
	int camera_num;
};
#endif


Visca_Ctrl::Visca_Ctrl(string device)
{
#ifdef VISCA_CAMCTRL
	internal = new LibviscaInternal;
	if (VISCA_open_serial(&internal->interface, (char*)device.c_str())!=VISCA_SUCCESS)
	{
        fprintf(stderr,"Visca_Ctrl: unable to open serial device %s\n",device.c_str());
		initialized=false;
    }
    else
    {
		internal->interface.broadcast=0;
		VISCA_set_address(&(internal->interface), &(internal->camera_num));
		internal->camera.address=1;
		VISCA_clear(&internal->interface, &internal->camera);

		VISCA_get_camera_info(&internal->interface, &internal->camera);

		//	if (VISCA_set_zoom_value(&interface, &camera, 0x0000)!=VISCA_SUCCESS)
		//		fprintf(stderr,"error setting zoom\n");

        timeoutProvider = new Timeout_Provider<int, Visca_Ctrl*>;
		initialized=true;
	}
#endif
}

Visca_Ctrl::~Visca_Ctrl()
{
#ifdef VISCA_CAMCTRL
    if (initialized)
    {
        timeoutProvider->stop_thread();
		delete internal;
	}
#endif
}

void Visca_Ctrl::timeout(int)
{
#ifdef VISCA_CAMCTRL
	if (!initialized)
		return;

    if (VISCA_set_zoom_stop(&internal->interface, &internal->camera) != VISCA_SUCCESS)
		return;
    else if (VISCA_set_pantilt_stop(&internal->interface, &internal->camera, 0, 0) != VISCA_SUCCESS)
		return;
#endif
}

/**
Input: -128..127
Output: 0..max (absolute value)
 */
int Visca_Ctrl::map_speed(int charVal, int intervalMin, int intervalMax)
{
	if (charVal==0)
		return 0;
	int max = intervalMax-intervalMin;

	int cmax=127;
    if (charVal<0)
    {
        charVal =- charVal;
        cmax = 128;
	}
    int ret = charVal*max/cmax + intervalMin;
	if (ret<intervalMin)	 
		return intervalMin;
	else
		return ret;
}

void Visca_Ctrl::set_pan_tilt_zoom(int pspeed, int tspeed, int zspeed, int durationMs)
{
#ifdef VISCA_CAMCTRL
	if (!initialized)
		return;

    timeoutProvider->cancel_request(this,0);

//	cerr <<"VISCA::SETPANTILTZOOM: "<<pspeed<<","<<tspeed<<","<<zspeed<<" "<<durationMs<<endl;

	if (durationMs<50)
		durationMs=50;

    bool fail = false;

    if (zspeed>0)
    {
        if (VISCA_set_zoom_tele_speed (&internal->interface, &internal->camera, map_speed(zspeed,2,7)) != VISCA_SUCCESS)
            fail = true;
	}
    if (zspeed<0)
    {
        if (VISCA_set_zoom_wide_speed (&internal->interface, &internal->camera, map_speed(zspeed,2,7)) != VISCA_SUCCESS)
            fail = true;
	}
    if (zspeed==0)
    {
        if (VISCA_set_zoom_stop(&internal->interface, &internal->camera) != VISCA_SUCCESS)
            fail = true;
	}

    if (pspeed==0&&tspeed==0)
    {
		//cerr <<"EEEE: VISCA: STOP"<<endl;
        if (VISCA_set_pantilt_stop(&internal->interface, &internal->camera, 0, 0) != VISCA_SUCCESS)
            fail = true;
	}

    if (pspeed>0 && tspeed==0)
    { // pan speed only
		//cerr <<"EEEE: VISCA: right "<< mapSpeed(pspeed,0,24) <<" 0..24" <<endl;
        if (VISCA_set_pantilt_right(&internal->interface, &internal->camera, map_speed(pspeed,0,24),0) != VISCA_SUCCESS)
			fail=true;
    }
    else if (pspeed<0 && tspeed==0)
    {
		//cerr <<"EEEE: VISCA: left "<< mapSpeed(pspeed,0,24) <<" 0..24" <<endl;
        if (VISCA_set_pantilt_left(&internal->interface, &internal->camera, map_speed(pspeed,0,24),0) != VISCA_SUCCESS) {
			fail=true;
		}
	}else if (pspeed==0 && tspeed>0){ // tilt speed only
		//cerr <<"EEEE: VISCA: up "<< mapSpeed(tspeed,0,20) <<" 0..20" <<endl;
        if (VISCA_set_pantilt_up(&internal->interface, &internal->camera, 0, map_speed(tspeed,0,20)) != VISCA_SUCCESS) {
			fail=true;
		}
	}else if (pspeed==0 && tspeed<0){
		//cerr <<"EEEE: VISCA: down "<< mapSpeed(tspeed,0,20) <<" 0..20" <<endl;
        if (VISCA_set_pantilt_down(&internal->interface, &internal->camera, 0, map_speed(tspeed,0,20)) != VISCA_SUCCESS) {
			fail=true;
		}
	}else if (pspeed>0 && tspeed>0){ // up right
		//cerr <<"EEEE: VISCA: upright "<< mapSpeed(pspeed,0,24) <<" " << mapSpeed(tspeed,0,20) <<" 0..24/20" <<endl;
        if (VISCA_set_pantilt_upright(&internal->interface, &internal->camera, map_speed(pspeed,0,24), map_speed(tspeed,0,20)) != VISCA_SUCCESS) {
			fail=true;
		}
	}else if (pspeed>0 && tspeed<0){ // down right
		//cerr <<"EEEE: VISCA: downright "<< mapSpeed(pspeed,0,24) << " "<< mapSpeed(tspeed,0,20) <<" 0..24/20" <<endl;
        if (VISCA_set_pantilt_downright(&internal->interface, &internal->camera, map_speed(pspeed,0,24), map_speed(tspeed,0,20)) != VISCA_SUCCESS) {
			fail=true;
		}
	}else if (pspeed<0 && tspeed<0){ // down left
		//cerr <<"EEEE: VISCA: downleft"<< mapSpeed(pspeed,0,24) << " "<< mapSpeed(tspeed,0,20) <<" 0..24/20" <<endl;
        if (VISCA_set_pantilt_downleft(&internal->interface, &internal->camera, map_speed(pspeed,0,24), map_speed(tspeed,0,20)) != VISCA_SUCCESS) {
			fail=true;
		}

	}else if (pspeed<0 && tspeed>0){ // up left
		//cerr <<"EEEE: VISCA: upleft"<< mapSpeed(pspeed,0,24) << " "<< mapSpeed(tspeed,0,20) <<" 0..24/20" <<endl;
        if (VISCA_set_pantilt_upleft(&internal->interface, &internal->camera, map_speed(pspeed,0,24), map_speed(tspeed,0,20)) != VISCA_SUCCESS) {
			fail=true;
		}
	}

    if (fail)
    {
        cerr <<"WARNING: Visca_Ctrl::set_pan_tilt_zoom: could not communicate with camera"<<endl;
	}else{
        timeoutProvider->request_timeout(durationMs, this, 0);
	}
#endif
}
