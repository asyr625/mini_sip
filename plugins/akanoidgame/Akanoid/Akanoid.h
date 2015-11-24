/*
 * Akanoid.h
 *
 *  Created on: Apr 18, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#ifndef AKANOID_H_
#define AKANOID_H_


#include <vector>
#include "./GameEngine/Scene.h"
#include "./GameEngine/CircleNode.h"
#include "./GameEngine/TextNode.h"
#include "./Util/PlayerScoreData.hpp"

#include "Ball.h"
#include "Paddle.h"
#include "Block.h"
#include "LeaderBoard.h"
#include "ScoreBoard.h"
#include "Item.h"

using namespace GameEngine;


class Akanoid: public GameEngine::Scene
{

private:

	// for record statistic
	int _playerStatistic;

	int _nextItemCount;
	int _currentItemCount;

	unsigned int _labelDisplayDuration;

	OpenGlWindow *_renderWindow;

	std::vector<PlayerScoreData> _highScoreData;

	std::vector<Item> _items;
	std::vector<Ball> _balls;
	Paddle _paddle;

	TextNode _scoreLabel;
	TextNode _ballLabel;
	TextNode _itemInfoLabel;
	TextNode _touchToStartLabel;

	LeaderBoard _leaderBoard;
	ScoreBoard _scoreBoard;

	std::vector<Block> _blocks;
	std::string _playerName;

	bool _isPlaying;
	bool _hasTopScoreUpdate;

	unsigned int _currentScore;
	unsigned int _lastScore;
	int _state;
	int _level;
	int _playerLife;

	void loadConfigureFile();
	std::string getDataPath();
	void loadHighScoreData();
	void createGamePackage();

	void setScoreLabel(int score);
	void setLifeLabel(int life);

	void gameLoading();
	void gameLevelCompleted();
	void gameReset();
	void gameOver();



	void checkPaddleCollistion(Ball &ball);
	void checkBlockCollision(Ball &ball);
	void checkItemCollision();

	void saveScore();
	void saveHiScore(std::string name, std::string date , std::string time , unsigned int score , bool islocalPlayer);


	void createNewLevel();
	void createBall();
	void createItemBoxAt(int x , int y);


	void setGameState(int state);



public:

	Akanoid();
	virtual ~Akanoid();


	void setRenderWindow(OpenGlWindow *window);
	void sendNetworkDone();
	bool hasTopScoreUpdate();
	int playStat();


    void enterScene();
    void leaveScene();

	void draw();


	void update(double elapsed);
	void onMouseClick(int x , int y);
	void onMouseMove(int x, int y);
	void onKeyDown(std::string key);
	void receivedNetworkData(GamePackage *package);


};

#endif /* AKANOID_H_ */
