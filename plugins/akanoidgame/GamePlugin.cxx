
/*
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include <unistd.h>
#include <libmutil/stringutils.h>
#include <libmutil/Thread.h>
#include <libmutil/mtime.h>
#include <minisip/Animate.h>
#include <minisip/Text.h>
#include <minisip/OpenGlWindow.h>
#include <math.h>
#include <GL/glut.h>
#include <fstream>
#include <list>
#include <iostream>

#include "Akanoid/Util/GamePackage.h"
#include "Akanoid/Util/GameRtcpReport.h"
#include "Akanoid/GameEngine/Engine.h"

using namespace std;

static int initialized=false;
static std::list<std::string> pluginList;


RtcpReport* rtcpFactory(unsigned char* buf, int buflen)
{
	if (buf[0]!='G' || buf[1]!='A' || buf[2]!='M' || buf[3]!='E')
		return NULL;

	return new GameRtcpReport(buf , buflen);
}



class AkanoidGame: public OpenGlUi, public RtcpCallback /*, public Runnable*/{

	private:
		bool initialized;
		mgl_gfx* iconTexture;
		mgl_gfx* iconUpdateTexture;

		bool showApp;
		bool showWidget;
		GameEngine::Engine akanoidGame;
		bool isGameEngineStarted;


	public:
		AkanoidGame(OpenGlWindow* w):OpenGlUi(w)
	{
			iconTexture=0;
			iconUpdateTexture = 0;
			enabled=true;
			initialized=false;
			showApp=showWidget=false;

			RtcpPacket::addRtcpFactory( rtcpFactory );
			RtcpMgr::addRtcpCallbackGlobal( this );

			isGameEngineStarted = false;

		}

		virtual ~AkanoidGame(){}
		virtual void init(){}
		virtual void stop(){}
		virtual bool hasLayoutManager(){return false;}
		virtual bool hasApp(){return true;}
		virtual bool hasWidget(){return true;}
		virtual bool hasIcon(){return true;}
		virtual bool hasVideoDecoration(){return false;}
		virtual bool appVisible(){return showApp;}
		virtual bool widgetVisible(){return showWidget;}

		virtual void setAppVisible(bool v)
		{
			showApp=v;
			if( v == true)
			{
				// user enter game
				startEngine();
			}
			else
			{
				// user leave game
				if(akanoidGame.isGameRunning())
					akanoidGame.stopGame();
			}

		}
		virtual void setWidgetVisible(bool v){showWidget=v;}
		virtual bool supportsTouch(){return false;}
		virtual void touch(MTouch &mt){}
		virtual bool getDefaultEnabled(){return true;}
		virtual bool mouseMove(int sx, int sy, float glx,float gly)
		{
			if(akanoidGame.isGameRunning())
				akanoidGame.onMouseMove(sx,sy);
			return false;
		}
		virtual bool mouseDown(int sx, int sy, float glx, float gly, int button){return false;}
		virtual bool mouseUp(int sx, int sy, float glx, float gly, int button){return false;}
		virtual bool mouseClick(int sx, int sy, float glx, float gly, int button)
		{
			if(akanoidGame.isGameRunning())
				akanoidGame.onMouseClick(sx,sy);

			return false;
		}
		virtual bool mouseDoubleClick(int sx, int sy, float glx, float gly, int button){ return false;}
		virtual bool mouseWheelUp(){return false;}
		virtual bool mouseWheelDown(){return false;}

		virtual bool mouseDragMove(int from_sx, int from_sy,
					float from_glx, float from_gly,
					int cur_sx, int cur_sy,
					float cur_glx, float cur_gly,
					int button)
		{
			return false;
		}

		virtual bool mouseDragMove(int from_sx, int from_sy,
					float from_glx, float from_gly,
					int cur_sx, int cur_sy,
					float cur_glx, float cur_gly,
					int button,
					int delta_sx, int delta_sy,
					float delta_glx, float delta_gly)
		{
			return false;
		}

		virtual bool mouseDragRelease(int from_sx, int from_sy, float from_glx, float from_gly,  int cur_sx, int cur_sy, float cur_glx, float cur_gly, int button){return false;}
		virtual bool consumekey(SDL_KeyboardEvent& key)
		{
			if(akanoidGame.isGameRunning())
				akanoidGame.onKeyInput(key);

			return false;
		}
		virtual void drawBackground(){}
		virtual void drawWidget(){}
		virtual void drawIcon(){
			uint64_t now = mtime();
			if (!initialized)
			{
				initialized=true;
				iconTexture = window->loadTexture("akanoid",256);
				iconUpdateTexture = window->loadTexture("akanoid_up",256);
			}
			if (iconTexture && iconUpdateTexture)
			{

				if(akanoidGame.hasUpdate())
					window->drawTexture(iconUpdateTexture, iconBounds.getX1(now), iconBounds.getY1(now), iconBounds.getX2(now), iconBounds.getY2(now), 0.0, iconAlpha->getVal(now), 0);
				else
					window->drawTexture(iconTexture, iconBounds.getX1(now), iconBounds.getY1(now), iconBounds.getX2(now), iconBounds.getY2(now), 0.0, iconAlpha->getVal(now), 0);

			}
		}


		virtual void drawApp()
		{
			// draw App
			akanoidGame.draw();
		}

		virtual void drawVideoDecoration(OpenGlDisplay* display){}
		virtual void postDraw(){}
		virtual void updateLayout(float x1, float y1, float x2, float y2, uint64_t cur_time, AnimationType anim){}
		virtual void draw(uint64_t curTime){}
		virtual void addDisplay(MRef<OpenGlDisplay*> d){}
		virtual void removeDisplay(MRef<OpenGlDisplay*> d){}

		//-------------------------------------------------------------
		//	ENGINE CONTROL
		//-------------------------------------------------------------
		void startEngine()
		{
			if(isGameEngineStarted == false)
			{
				akanoidGame.setRenderWindow(window);
				akanoidGame.setWindowSize(window->getWindowWidth(), window->getWindowHeight());
				akanoidGame.startGame();
				isGameEngineStarted = true;
			}
			else
			{
				akanoidGame.setWindowSize(window->getWindowWidth(), window->getWindowHeight());
				akanoidGame.startGame();
			}

		}

		//-------------------------------------------------------------
		//	NETWORK
		//-------------------------------------------------------------
		virtual bool handleRtcp(const MRef<RtcpPacket*>& rtcp)
		{
			std::cout << "\n\nGet package \n\n" << std::endl;
			std::vector<RtcpReport *> reports = rtcp->getReports();
			std::vector<RtcpReport *>::iterator i;
			for (i=reports.begin(); i!=reports.end(); i++)
			{

				if(dynamic_cast<GameRtcpReport*>(*i))
				{
					std::cout << "\nGame..data" << std::endl;
					GameRtcpReport* r = (GameRtcpReport*) *i;
					GamePackage *p = r->getGamePackage();
					if( p!= NULL)
						akanoidGame.receivedNetworkData(p);
					return true;
				}
			}
			return false;
		}

};



//-------------------------------------------------------------
//	Plug-in class and function
//-------------------------------------------------------------


class AkanoidGamePlugin : public OpenGlUiPlugin{
	public:
		AkanoidGamePlugin(MRef<Library *> lib){
			seen = false;
		}

		MRef<OpenGlUi*> newInstance( OpenGlWindow* w){
			return new AkanoidGame(w);

		}

		virtual std::string getMemObjectType() const {return "AkanoidGamePlugin";}

		virtual std::string getName() const{ return "AkanoidGame";}
		virtual uint32_t getVersion() const{return 1;}
		virtual std::string getDescription() const{return "AkanoidGame overlay plugin";}
		virtual std::string getPluginType() const{return "openglui";}

};


extern "C"
std::list<std::string> *akanoidgame_LTX_listPlugins( MRef<Library*> lib ){
	if( !initialized ){
		pluginList.push_back("getPlugin");
		initialized = true;
	}

	return &pluginList;
}

extern "C"
MPlugin * akanoidgame_LTX_getPlugin( MRef<Library*> lib ){
	return new AkanoidGamePlugin( lib );
}


