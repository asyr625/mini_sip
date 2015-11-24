/*
 * Engine.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */
#include <fstream>
#include "Engine.h"
#include "../Util/GameRtcpReport.h"
#include "../Util/pugixml.hpp"

#define WINDOW_WIDTH 768
#define WINDOW_HEIGHT 432


namespace GameEngine
{

Engine::Engine()
:
 _width(WINDOW_WIDTH),
 _height(WINDOW_HEIGHT),
 _x(0),
 _y(0),
 _isPlaying(false),
 _renderWindow(0)
{
	_renderWindow = 0;
	_currentTime = 0;
	_lastTime = 0;

	_engineStartTime = 0;

	gScreenWidth = _width;
	gScreenHeight = _height;

	_screenWidth = WINDOW_WIDTH;
	_screenHeight = WINDOW_HEIGHT;

}


Engine::Engine(int w = 800, int h = 600, int x = 0, int y = 0)
:
 _width(w),
 _height(h),
 _x(x),
 _y(y),
 _isPlaying(false),
 _renderWindow(0)
{


	_renderWindow = 0;
	_currentTime = 0;
	_lastTime = 0;
	_engineStartTime = 0;

	_screenWidth = WINDOW_WIDTH;
	_screenHeight = WINDOW_HEIGHT;

}


Engine::~Engine() {
}



void Engine::setRenderWindow(OpenGlWindow* window)
{
	_renderWindow = window;
	_aka.setRenderWindow(window);
}

void Engine::setWindowSize(int width, int height)
{
	_screenWidth = width;
	_screenHeight = height;

}


void Engine::onKeyInput(SDL_KeyboardEvent& key)
{
	//SDLMod modifier = key.keysym.mod;
	printf("%s \n", SDL_GetKeyName(key.keysym.sym));
	std::string character(SDL_GetKeyName(key.keysym.sym) );
	if( character == "a")
		sendGamePackage();
	else
		_aka.onKeyDown(character);

}
void Engine::onMouseClick(int x, int y)
{

	float px = x * ((float)_width / (float)_screenWidth) ;
	float py = y * ((float)_height / (float)_screenHeight) ;

	_aka.onMouseClick(px,py);

}

void Engine::onMouseMove(int x, int y)
{
	float px = x * ((float)_width / (float)_screenWidth) ;
	float py = y * ((float)_height / (float)_screenHeight) ;

	_aka.onMouseMove(px,py);
}



void Engine::enter2DSpace()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, _width, _height, 0, 0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void Engine::leave2DSpace()
{

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void Engine::draw()
{
	if(_isPlaying == false)
		return;
	else
	{

		_currentTime = SDL_GetTicks();
		unsigned int diff = _currentTime - _lastTime;
		_lastTime = _currentTime;

		_screenWidth = _renderWindow->getWindowWidth();
		_screenHeight = _renderWindow->getWindowHeight();

		// Update game
		_aka.update(diff);

		// Render
		enter2DSpace();
		_aka.draw();
		leave2DSpace();


		// Sending network package
		sendGamePackage();

    }

}

void Engine::startGame()
{


	time(&_engineStartTime);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	_isPlaying = true;

	_aka.enterScene();

	_lastTime = SDL_GetTicks();


}
void Engine::stopGame()
{
	_aka.leaveScene();
	_isPlaying = false;
	SDL_EnableKeyRepeat(0 , SDL_DEFAULT_REPEAT_INTERVAL);

	recordPlayerStatistic();

}

void Engine::recordPlayerStatistic()
{
	struct tm* timeinfo;
	char buffer[80] = {0};
	timeinfo = localtime(&_engineStartTime);
	strftime(buffer,80,"%d-%m-%Y  %I:%M:%S",timeinfo);
	std::string timeDateString(buffer);


	double totalTime = std::difftime(std::time(NULL), _engineStartTime);

    std::string logpath = getenv("HOME");
    logpath +="/.minisip/statistic.xml";

    pugi::xml_document doc;
    doc.load_file(logpath.c_str());

    // local score
    pugi::xml_node localNode = doc.child("gameStatistic");
    if(localNode.empty())
    	localNode = doc.append_child("gameStatistic");

    pugi::xml_node stat = localNode.append_child("stat");
    stat.append_attribute("date") = timeDateString.c_str();
    stat.append_attribute("playStatus") = _aka.playStat();
    stat.append_attribute("usageTime") = totalTime;

    doc.save_file(logpath.c_str());



}

bool Engine::hasUpdate()
{
	return _aka.hasTopScoreUpdate();
}

bool Engine::isGameRunning()
{
	return _isPlaying;
}


void Engine::receivedNetworkData(GamePackage *package)
{
	_aka.receivedNetworkData(package);
}

void Engine::sendGamePackage()
{

	int size = _aka.getPackage().size();
	if( size == 0)
		return;

	MRef<SessionRegistry*> sreg = _renderWindow->getSessionRegistry();
	std::list<MRef<Session*> > sessions = sreg->getAllSessions();

//	sessions.begin();


	std::list< MRef<RealtimeMediaStreamSender*> > senders = (*sessions.begin())->getRealtimeMediaStreamSenders();

	MRef<RealtimeMediaStreamSender*> sender;

	std::list< MRef<RealtimeMediaStreamSender*> >::iterator i;
	for (i=senders.begin(); i!=senders.end(); i++)
	{
		if ((*i)->getRealtimeMedia()->getSdpMediaType()=="video")
			sender=*i;
	}

	MRef<RtcpPacket*> rtcp = new RtcpPacket;

	for (int i = 0; i < size ; i++)
	{
		GamePackage p = _aka.getPackage()[i];
		rtcp->addReport (new GameRtcpReport(p));
	}

	// sending package 10x
	for( int n = 0 ; n < 10; n++)
		sender->sendRtcpPacket(rtcp);

	// clear package
	_aka.sendNetworkDone();

}
//
//void Engine::setPlayerName(std::string playerName)
//{
//	_aka.setPlayerName(playerName);
//}


}
