/*
 * Scene.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */


#include "../Util/GamePackage.h"
#include "Node.h"

#ifndef SCENE_H_
#define SCENE_H_

namespace GameEngine
{

class Scene : public Node {

protected:
	std::vector<GamePackage> _package;
	//std::vector<std::string> _infoList;


private:
    int _sceneID;
public:
	Scene(std::string name);
	Scene(std::string name,int sceneID);
	virtual ~Scene();
    virtual void update(unsigned int elapsedTime) { Node::update(elapsedTime);}

    virtual void enterScene();
    virtual void leaveScene();

    int getSceneID();
    void setScenID(int sceneID);
    
//    std::string &infoString();
//    std::vector<std::string> &infoList();

    std::vector<GamePackage> &getPackage();

    virtual void receivedNetworkData(GamePackage *package);
};

}

#endif /* SCENE_H_ */
