/*
 * GameWindow.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include <minisip/OpenGlWindow.h>

#include "CircleNode.h"
#include "RectangleNode.h"
#include "../Util/GamePackage.h"
#include "../../Akanoid/Akanoid.h"

#ifndef GAMEWINDOW_H_
#define GAMEWINDOW_H_

namespace GameEngine {

class Engine {

private:
	int _width;
	int _height;

	int _screenWidth;
	int _screenHeight;

	int _x;
	int _y;
	bool _isPlaying;
	OpenGlWindow* _renderWindow;

	Akanoid _aka;

	unsigned int _currentTime;
	unsigned int _lastTime;

	time_t _engineStartTime;

	void enter2DSpace();
	void leave2DSpace();
	void sendGamePackage();

	void recordPlayerStatistic();




public:
	virtual ~Engine();

	Engine();
	Engine(int w , int h ,int x, int y);

	void startGame();
	void stopGame();
	bool isGameRunning();

	void setRenderWindow(OpenGlWindow* window);

	void draw();
	void setWindowSize(int width, int height);

	void onKeyInput(SDL_KeyboardEvent& key);
    void onMouseClick(int x, int y);
    void onMouseMove(int x, int y);

	void receivedNetworkData(GamePackage *package);
	bool hasUpdate();

};

}
#endif /* GAMEWINDOW_H_ */
