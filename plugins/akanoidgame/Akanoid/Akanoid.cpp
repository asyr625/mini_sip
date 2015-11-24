/*
 * Akanoid.cpp
 *
 *  Created on: Apr 18, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include "Akanoid.h"

#include "./Util/pugixml.hpp"
//#include <algorithm>


static bool OrderByScore(const PlayerScoreData& data1,const  PlayerScoreData& data2)
{
     return data1.score > data2.score ;
}

template<class T1, class T2> bool isIntersecting(T1& mA, T2& mB)
{
	return mA.right() >= mB.left()
			&& mA.left() <= mB.right()
			&& mA.bottom() >= mB.top()
			&& mA.top() <= mB.bottom();
}


#define PACKAGE_UPDATE_SCORE 2

#define GAME_HI_SCORE 1
#define GAME_READY	2
#define GAME_PLAYING 3
#define GAME_CLEAR 4
#define GAME_OVER 5
#define GAME_STOP 6


#define PLAYER_ENTER_GAME 1
#define PLAYER_NOT_FINISH_PLAYING 2
#define PLAYER_FINISH_PLAYING 3

const char *LOCAL_TAG = "localScore";
const char *HI_TAG = "hiScore";
const char *NET_TAG = "netScore";



Akanoid::~Akanoid()
{
}


Akanoid::Akanoid() : Scene("AkanoidScene", 0)
{
	_hasTopScoreUpdate = false;
	_isPlaying = false;
	_hasTopScoreUpdate = false;

	_currentScore = 0;
	_lastScore = 0;
	_state = 0;
	_level = 0;

	_playerLife = 0;
	_playerStatistic = 0;

	//_item = NONE;


	loadConfigureFile();
}

//////////////////////////////////////////////////////////////
//
//		UTIL
//
//////////////////////////////////////////////////////////////
void Akanoid::loadConfigureFile()
{
	srand(time(0));

	pugi::xml_document reader;
	std::string logpath = getenv("HOME");
	logpath+="/.minisip/minisip.conf";
	reader.load_file(logpath.c_str());

	const char* value = reader.root().child_value("sdes_location");
	if(value != NULL)
	{
		std::string str(value);

		char chars[] = "\n\t";
		for (unsigned int i = 0; i < strlen(chars); ++i)
		      str.erase (std::remove(str.begin(), str.end(), chars[i]), str.end());

		if(str.length() > 15)
		{
			unsigned end = str.rfind(" ");
			str = str.substr(0,end);
		}
		_playerName.assign(str);

	}

}

std::string Akanoid::getDataPath()
{
	std::string logpath = getenv("HOME");
    logpath +="/.minisip/akanoid.xml";
    return logpath;
}

bool Akanoid::hasTopScoreUpdate()
{
	return _hasTopScoreUpdate;
}

int Akanoid::playStat()
{
	return _playerStatistic;
}



//////////////////////////////////////////////////////////////
//
//		SETTING
//
//////////////////////////////////////////////////////////////

void Akanoid::setScoreLabel(int score)
{

	std::ostringstream sstream;
	sstream << "LV." << (_level + 1) << " : " << score;
	std::string message = sstream.str();
	_scoreLabel.setText(message);

}

void Akanoid::setLifeLabel(int life)
{
	std::ostringstream stream;
	stream << "Life: " << life;
	_ballLabel.setText(stream.str());
}


void Akanoid::setRenderWindow(OpenGlWindow *window)
{
	_renderWindow = window;
	_scoreLabel.setRenderWindow(window);
	_leaderBoard.setRenderWindow(window);
	_scoreBoard.setRenderWindow(window);
	_ballLabel.setRenderWindow(window);
	_itemInfoLabel.setRenderWindow(window);
}


//////////////////////////////////////////////////////////////
//
//		KEYBOARD & MOUSE
//
//////////////////////////////////////////////////////////////
void Akanoid::onMouseMove(int x, int y)
{
	if(_state == GAME_PLAYING)
	{
		float y = _paddle.y();
		_paddle.setPosition(x,y);
	}
}

void Akanoid::onMouseClick(int x , int y)
{


	switch(_state)
	{
		case GAME_HI_SCORE:
			setGameState(GAME_READY);
			break;
		case GAME_READY:
			setGameState(GAME_PLAYING);
		break;
		case GAME_CLEAR:
			setGameState(GAME_READY);
		break;
		case GAME_OVER:
			setGameState(GAME_READY);
		break;
		case GAME_STOP:
			setGameState(GAME_PLAYING);
		break;
		//		case GAME_PLAYING:
		//		{
		//			int mod = x%4;
		//			_paddle.moveTo(x - mod);
		//		}
		//		break;
	}

}

void Akanoid::onKeyDown(std::string key)
{

//	if( key == "c")
//	{
//		_ballList[0].doubleSize(10);
//	}

}



//////////////////////////////////////////////////////////////
//
//		ENTER - LEAVE GAME
//
//////////////////////////////////////////////////////////////

void Akanoid::enterScene()
{
	_blocks.clear();
	_balls.clear();
	_items.clear();
	gameLoading();
	setGameState(GAME_HI_SCORE);

	_nextItemCount = (rand() % 10) + 5;
	_playerLife = 0;
	_labelDisplayDuration = SDL_GetTicks();
	//_level = 5;
}

void Akanoid::gameLoading()
{


	_hasTopScoreUpdate = false;
	_currentScore = 0;
	_playerStatistic = PLAYER_ENTER_GAME;
	_level = 0;
	_playerLife = 2;

	_scoreLabel.setColor(ColorRGBA(0,0,0));
	_scoreLabel.setFontSize(20);
	_scoreLabel.setText("LV.1 : 0");
	_scoreLabel.setPosition(blockWidth+10,blockHeight - 10 );

	_itemInfoLabel.setColor(ColorRGBA(0,0,0));
	_itemInfoLabel.setFontSize(30);
	_itemInfoLabel.setText("Touch to begin");
	_itemInfoLabel.setPosition(gScreenWidth/2.0 - 80,gScreenHeight/2.0);
	_itemInfoLabel.setVisible(true);

	_ballLabel.setColor(ColorRGBA(0,0,0));
	_ballLabel.setFontSize(20);
	_ballLabel.setText("Life: 2");
	_ballLabel.setPosition(gScreenWidth - 140,blockHeight - 10 );


	_leaderBoard.setSize(520,360);
	_leaderBoard.setPosition(gScreenWidth/2.0,10 );
	_leaderBoard.setScore(_lastScore);
	_leaderBoard.setPlayerName(_playerName);

	_scoreBoard.setSize(400,100);
	_scoreBoard.setPosition(gScreenWidth/2.0,40 );


	_scoreLabel.setVisible(false);
	_scoreBoard.setVisible(false);
	_leaderBoard.setVisible(false);

	_paddle.setVisible(false);

}


void Akanoid::leaveScene()
{
	_isPlaying = false;
}


//////////////////////////////////////////////////////////////
//
//		GAME STATE AND CONTROL
//
//////////////////////////////////////////////////////////////


void Akanoid::setGameState(int state)
{
	_state = state;

	switch(state)
	{
		case GAME_HI_SCORE:
			loadHighScoreData();
			_isPlaying = false;
			_scoreLabel.setVisible(false);
			_paddle.setVisible(false);
			_scoreBoard.setVisible(false);
			_ballLabel.setVisible(false);
			_itemInfoLabel.setVisible(false);
		    _leaderBoard.setVisible(true);

		break;
		case GAME_READY:
			createNewLevel();
			_itemInfoLabel.setVisible(true);
			_isPlaying = false;
		break;
		case GAME_PLAYING:
			_isPlaying = true;
			if(_playerStatistic == PLAYER_ENTER_GAME)
				_playerStatistic = PLAYER_NOT_FINISH_PLAYING;
		break;

		case GAME_STOP:
			_isPlaying = false;
			gameReset();

		break;

		case GAME_CLEAR:
			gameLevelCompleted();
			_balls.clear();
		    _leaderBoard.setVisible(false);
			_scoreLabel.setVisible(false);
			_paddle.setVisible(false);
			_ballLabel.setVisible(false);
			_scoreBoard.setVisible(true);

		break;
		case GAME_OVER:
			gameOver();
			_scoreLabel.setVisible(false);
			_paddle.setVisible(false);
			_scoreBoard.setVisible(false);
			_ballLabel.setVisible(false);
		    _leaderBoard.setVisible(true);

		break;
	}
}


void Akanoid::createNewLevel()
{

	_itemInfoLabel.setText("Touch to begin");
	_labelDisplayDuration = 0;
	_currentItemCount = 0;
	_nextItemCount = (rand() % 10) + 5;

	if(_playerLife <= 0 )
		_playerLife = 2;

	int row = 7;
	int col = 5;

	// Generate blocks
	_blocks.clear();
	for(int r = 0; r < row; r++)
	{
		for( int c = 0; c < col; c++ )
		{
			int x = (r + 1) * (blockWidth + 3) + 32;
			int y = (c + 2) * (blockHeight + 3);
			_blocks.push_back(Block(x,y));
		}
	}



	// Generate block level
	int count = _level * 3;
	while(count > 0)
	{
		int blockPos = rand() % (row * col);
		if(_blocks[blockPos].increseLevel())
			count--;
	}


	_paddle.setPosition(gScreenWidth/2.0, gScreenHeight - 20);
	_paddle.reset();

	setScoreLabel(_currentScore);
	setLifeLabel(_playerLife);

	_paddle.setVisible(true);

	_scoreLabel.setVisible(true);
	_ballLabel.setVisible(true);

	_scoreBoard.setVisible(false);
	_leaderBoard.setVisible(false);


	_balls.clear();
	createBall();

}

void Akanoid::createBall()
{
	Ball b;
	float x = _paddle.x();
	float y = _paddle.y() - 20;

	b.setPosition(x,y);

	b.start();
	b.setLevel(_level);
	_balls.push_back(b);
}

void Akanoid::gameOver()
{
	_isPlaying = false;
	_lastScore = _currentScore;
	_leaderBoard.setScore(_currentScore);

	// save score
	saveScore();

	// update top score
	loadHighScoreData();

	// send network package
	createGamePackage();

	_currentScore = 0;
	_level = 0;

}


void Akanoid::gameLevelCompleted()
{

	_isPlaying = false;

	_blocks.clear();

	_level++;

	_scoreBoard.setScoreLevel(_currentScore,_level);
	_scoreBoard.setVisible(true);

	_scoreLabel.setVisible(false);

	_paddle.setVisible(false);

}


void Akanoid::gameReset()
{

	setLifeLabel(_playerLife);
	_paddle.setPosition(gScreenWidth/2.0, gScreenHeight - 20);
	_labelDisplayDuration = 0;
	_itemInfoLabel.setText("Touch to begin");
	_itemInfoLabel.setVisible(true);

}

//////////////////////////////////////////////////////////////
//
//		GAME LOOP & LOGIC
//
//////////////////////////////////////////////////////////////

void Akanoid::update(double elapsed)
{

	if( _isPlaying == false)
		return;



	checkItemCollision();


	_paddle.update(elapsed);

	int size = _items.size();
	for( int i = 0 ; i < size ; i++)
		_items[i].update(elapsed);


	int maxBall = _balls.size();
	int deadCount = 0;
	for(int i = 0; i < maxBall;i++)
	{
		checkPaddleCollistion(_balls[i]);
		checkBlockCollision(_balls[i]);
		_balls[i].update(elapsed);
		if(_balls[i].isDead() == true)
			deadCount++;
	}

	if( maxBall != 0 && deadCount == maxBall)
	{
		_balls.clear();
		_paddle.reset();
		_items.clear();

		_playerLife--;

		if(_playerLife < 0)
		{
			setGameState(GAME_OVER);
		}
		else
		{
			setGameState(GAME_STOP);
			createBall();
		}
	}

	if(_isPlaying == true)
	{
		unsigned int timeNow = SDL_GetTicks();
		if( timeNow > _labelDisplayDuration + 2000)
			_itemInfoLabel.setVisible(false);
		else
			_itemInfoLabel.setVisible(true);
	}

}


void Akanoid::draw()
{
		// draw block
	int max = _blocks.size();
	for (int i = 0; i < max ; i++)
	{
			if(_blocks[i].isAlive() == true)
				_blocks[i].draw();
	}

	_paddle.draw();
	_scoreLabel.draw();
	_ballLabel.draw();

	_scoreBoard.draw();
	_leaderBoard.draw();

	_itemInfoLabel.draw();

	int size = _items.size();
	for( int i = 0 ; i < size ; i++)
		_items[i].draw();


	int maxBall = _balls.size();
	for(int i = 0; i < maxBall;i++)
		_balls[i].draw();

}


void Akanoid::checkItemCollision()
{
	int size = _items.size();
	for( int i = 0 ; i < size ; i++)
	{
		if(_items[i].isAlive())
		{
			if(!isIntersecting(_paddle, _items[i]))
				continue;

			_items[i].destroy();
			_labelDisplayDuration = SDL_GetTicks();

			switch(_items[i].getItemProperty())
			{
				case LONG_PADDLE:
					_paddle.doubleSize(60);
					_itemInfoLabel.setText("2x Paddle");
				break;
				case MULTIPLE_BALL:
					createBall();
					_itemInfoLabel.setText("+1 Ball");
				break;
				case BONUS_SCORE:
					_currentScore+= ((_level + 1) * 1000);
					setScoreLabel( _currentScore);
					_itemInfoLabel.setText("Bonus Score");
				break;
				case LIFE:
					_playerLife++;
					setLifeLabel(_playerLife);
					_itemInfoLabel.setText("+1 Life");
					break;
				case SUPER_BALL:
				{
					_itemInfoLabel.setText("Super Ball");
					int maxBall = _balls.size();
					for(int i = 0; i < maxBall;i++)
						_balls[i].doubleSize(40);

				}
				break;
			}
		}

	}


}

void Akanoid::checkPaddleCollistion(Ball &ball)
{
	if(!isIntersecting(_paddle, ball))
		return;


	// push upward if hit
	float xfactor = 0;
	float levelFactor = _level * 0.012;
	float velocity = (ballVelocity + levelFactor);

	int c = rand() % 2;
	if( c == 1)
		xfactor = 0.01;

	ball.velocity.y = -(velocity + xfactor) ;


	// set direction ( left , right )
	if(ball.x() < _paddle.x())
		ball.velocity.x = -velocity;
	else
		ball.velocity.x = velocity;

}



void Akanoid::createItemBoxAt(int x , int y)
{

	_currentItemCount++;
	if( _currentItemCount == _nextItemCount)
	{
		Item item;
		item.setRenderWindow(_renderWindow);
		item.setPosition(x,y);
		_items.push_back(item);

		_nextItemCount = (rand() % 17) + 5;
		_currentItemCount = 0;
	}
}

void Akanoid::checkBlockCollision(Ball &ball)
{
	int max = _blocks.size();
	if( max == 0)
		return;

	int destroyBlock = 0;

	for (int i = 0; i < max ; i++)
	{
		if(_blocks[i].isAlive() == false)
		{
			destroyBlock++;
			continue;
		}
		// not hit
		if(!isIntersecting(_blocks[i], ball))
			continue;

		_blocks[i].destroy();
		createItemBoxAt(_blocks[i].x(),_blocks[i].y());

		_currentScore += 100;
		this->setScoreLabel(_currentScore);


		float overlapLeft = ball.right() - _blocks[i].left();
		float overlapRight = _blocks[i].right() - ball.left();
		float overlapTop = ball.bottom() - _blocks[i].top();
		float overlapBottom = _blocks[i].bottom() - ball.top();

		bool ballFromLeft(abs(overlapLeft) < abs(overlapRight));
		bool ballFromTop(abs(overlapTop) < abs(overlapBottom));

		float minOverlapX = ballFromLeft ? overlapLeft : overlapRight;
		float minOverlapY = ballFromTop ? overlapTop : overlapBottom;

		float levelFactor = _level * 0.01;
		float velocity = (ballVelocity + levelFactor);

		if(abs(minOverlapX) < abs(minOverlapY))
			ball.velocity.x = ballFromLeft ? -velocity : velocity;
		else
			ball.velocity.y = ballFromTop ? -velocity : velocity;

	}


	if(destroyBlock == max )
	{
		setGameState(GAME_CLEAR);
		_items.clear();
	}
}


//////////////////////////////////////////////////////////////
//
//		GAME DATA HANDLE
//
//////////////////////////////////////////////////////////////

void Akanoid::loadHighScoreData()
{

	_highScoreData.clear();


    std::string logpath = getDataPath();

    _leaderBoard.setName(_playerName);

    pugi::xml_document reader;
    reader.load_file(logpath.c_str());
    pugi::xml_node highestNode = reader.child(HI_TAG);

    if(!highestNode.empty())
    {
        for (pugi::xml_node_iterator it = highestNode.begin(); it != highestNode.end(); ++it)
        {

    		PlayerScoreData playerData;
    		playerData.date.assign(it->attribute("date").value());
    		playerData.time.assign(it->attribute("time").value());
    		playerData.name.assign(it->attribute("name").value());
    		playerData.score = atol(it->attribute("score").value());

    		_highScoreData.push_back(playerData);

        }

    }

    // no data ,
    else
    {
    	// try to load from local or other data
    	pugi::xml_node netNode = reader.child(NET_TAG);
        for (pugi::xml_node_iterator it = netNode.begin(); it != netNode.end(); ++it)
        {

    		PlayerScoreData playerData;
    		playerData.date.assign(it->attribute("date").value());
    		playerData.time.assign(it->attribute("time").value());
    		playerData.name.assign(it->attribute("name").value());
    		playerData.score = atol(it->attribute("score").value());

    		_highScoreData.push_back(playerData);

        }


    	pugi::xml_node localNode = reader.child(LOCAL_TAG);
        for (pugi::xml_node_iterator it = localNode.begin(); it != localNode.end(); ++it)
        {

    		PlayerScoreData playerData;
    		playerData.date.assign(it->attribute("date").value());
    		playerData.time.assign(it->attribute("time").value());
    		playerData.name.assign(it->attribute("name").value());
    		playerData.score = atol(it->attribute("score").value());

    		_highScoreData.push_back(playerData);

        }

    }


    std::sort(_highScoreData.begin(), _highScoreData.end(), &OrderByScore);
    unsigned long size = _highScoreData.size();
    while (size > 3)
    {
        _highScoreData.pop_back();
        size = _highScoreData.size();
    }



    if(_highScoreData.size() > 0)
    	_leaderBoard.setHighScore(_highScoreData);



    /// LOCAL SCORE

    std::vector<PlayerScoreData> localHighScoreData;
	pugi::xml_node localNode = reader.child(LOCAL_TAG);
    for (pugi::xml_node_iterator it = localNode.begin(); it != localNode.end(); ++it)
    {

		PlayerScoreData playerData;
		playerData.date.assign(it->attribute("date").value());
		playerData.time.assign(it->attribute("time").value());
		playerData.name.assign(it->attribute("name").value());
		playerData.score = atol(it->attribute("score").value());

		localHighScoreData.push_back(playerData);
    }
    std::sort(localHighScoreData.begin(), localHighScoreData.end(), &OrderByScore);
    unsigned long localSize = localHighScoreData.size();
    while (localSize > 3)
    {
    	localHighScoreData.pop_back();
    	localSize = localHighScoreData.size();
    }

    if(localHighScoreData.size() > 0)
    	_leaderBoard.setLocalScore(localHighScoreData);




}

void Akanoid::saveScore()
{

	time_t rawTime;
	struct tm* timeinfo;
	char buffer[80] = {0};

	time(&rawTime);
	timeinfo = localtime(&rawTime);

	strftime(buffer,80,"%I:%M:%S",timeinfo);
	std::string time(buffer);

	strftime(buffer,80,"%d-%m-%Y",timeinfo);
	std::string date(buffer);



    std::string logpath = getDataPath();

    pugi::xml_document doc;
    doc.load_file(logpath.c_str());

    // local score
    pugi::xml_node localNode = doc.child("localScore");
    if(localNode.empty())
    	localNode = doc.append_child("localScore");

    pugi::xml_node playerNode = localNode.append_child("player");
    playerNode.append_attribute("date") = date.c_str();
    playerNode.append_attribute("time") = time.c_str();
    playerNode.append_attribute("name") = _playerName.c_str();
    playerNode.append_attribute("score") = _currentScore;

    doc.save_file(logpath.c_str());

    saveHiScore(_playerName,date,time,_currentScore, true);

}



void Akanoid::saveHiScore(std::string name, std::string date , std::string time , unsigned int score , bool islocalPlayer)
{
    std::string logpath = getDataPath();

    pugi::xml_document doc;
    doc.load_file(logpath.c_str());
    pugi::xml_node hiNode = doc.child(HI_TAG);


    // no data then create new data
    if(hiNode.empty())
    {
    	hiNode = doc.append_child(HI_TAG);
        pugi::xml_node playerNode = hiNode.append_child("player");
        playerNode.append_attribute("date") = date.c_str();
        playerNode.append_attribute("time") = time.c_str();
        playerNode.append_attribute("name") = name.c_str();
        playerNode.append_attribute("score") = score;

        if( islocalPlayer == false)
        	_hasTopScoreUpdate = true;
    }
    else
    {

    	bool isHigher = false;
        for( unsigned int i = 0; i < _highScoreData.size() ; i++)
        {
        	if( score >= _highScoreData[i].score)
        	{
            	isHigher = true;
            	break;
        	}
        }

        if(islocalPlayer == true)
        {
        	if(_highScoreData.size() < 3)
        		isHigher = true;
        }

    	// save new high score
    	if( isHigher == true)
    	{
    		// checking duplicate item
    		bool isDuplicate = false;
        	for( unsigned int i = 0; i < _highScoreData.size() ; i++)
        	{
        		if (name.compare(_highScoreData[i].name) ==  0 &&
            			time.compare(_highScoreData[i].time) == 0 &&
            			date.compare(_highScoreData[i].date) == 0 )
        		{
        			isDuplicate = true;
        			break;
        		}
        	}

        	if( isDuplicate == false)
        	{
        		PlayerScoreData data;
        		data.date.assign(date);
        		data.time.assign(time);
        		data.name.assign(name);
        		data.score = score;

        		_highScoreData.push_back(data);

        	    std::sort(_highScoreData.begin(), _highScoreData.end(), &OrderByScore);
        	    unsigned long size = _highScoreData.size();
        	    while (size > 3)
        	    {
        	        _highScoreData.pop_back();
        	        size = _highScoreData.size();
        	    }

        	    // remove old data
        	    doc.remove_child(HI_TAG);

        	    // add new node
        	    hiNode = doc.append_child(HI_TAG);

        	    for(unsigned int i = 0; i < _highScoreData.size() ; i++ )
        	    {
        	    	//hiNode.children("player");
    		        pugi::xml_node playerNode = hiNode.append_child("player");
    		        playerNode.append_attribute("date") = _highScoreData[i].date.c_str();
    		        playerNode.append_attribute("time") = _highScoreData[i].time.c_str();
    		        playerNode.append_attribute("name") = _highScoreData[i].name.c_str();
    		        playerNode.append_attribute("score") = _highScoreData[i].score;
        	    }



        	    if(islocalPlayer == false)
        	    	_hasTopScoreUpdate = true;
        	}
    	}
    }

    doc.save_file(logpath.c_str());

}

//////////////////////////////////////////////////////////////
//
//		NETWORK
//
//////////////////////////////////////////////////////////////

void Akanoid::sendNetworkDone()
{
	_package.clear();
}

void Akanoid::createGamePackage()
{
	int size = _highScoreData.size();
	if( size == 0)
		return;

	GamePackage p;

	p.writeByte('G');
	p.writeByte('A');
	p.writeByte('M');
	p.writeByte('E');

	p.writeByte(PACKAGE_UPDATE_SCORE);
	p.writeByte(size);

	for( int i = 0; i < size ; i++ )
	{
		p.writeString(_highScoreData[i].name);
		p.writeInt(_highScoreData[i].score);
		p.writeString(_highScoreData[i].date);
		p.writeString(_highScoreData[i].time);
	}

	_package.push_back(p);

}


void Akanoid::receivedNetworkData(GamePackage *package)
{
	// ensure package is correct

	package->readByte();
	package->readByte();
	package->readByte();
	package->readByte();

	unsigned char pID = package->readByte();
	if( pID == PACKAGE_UPDATE_SCORE)
	{

		unsigned char size = package->readByte();
		for( int i = 0; i < size ; i++ )
		{
			std::string name = package->readString();
			unsigned int score = package->readInt();
			std::string date = package->readString();
			std::string time = package->readString();

	        saveHiScore(name,date,time,score,false);


			std::cout << "Got package: " << name << " " << date << " " << time << std::endl;

		}

	}
}






