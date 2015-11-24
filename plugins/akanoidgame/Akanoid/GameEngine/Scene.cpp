/*
 * Scene.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: Jakrit Tamnakpho
 *      jakrit@kth.se
 */

#include "Scene.h"

namespace GameEngine
{

Scene::Scene(std::string name) : Node(name)
{
	_sceneID = getId();
}

Scene::Scene(std::string name,int sceneID)
: Node(name) ,
_sceneID(sceneID)
{
    
}


Scene::~Scene() {
	// TODO Auto-generated destructor stub
}

int Scene::getSceneID()
{
    return _sceneID;
}

void Scene::setScenID(int sceneID)
{
    _sceneID = sceneID;
}
//
//std::string &Scene::infoString()
//{
//	return _infoString;
//}
//
//std::vector<std::string> &Scene::infoList()
//{
//	return _infoList;
//}


std::vector<GamePackage> &Scene::getPackage()
{
	return _package;
}


void Scene::enterScene()
{
}

void Scene::leaveScene()
{
}


void Scene::receivedNetworkData(GamePackage *package)
{
}





// end
}


