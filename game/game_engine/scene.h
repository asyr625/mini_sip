#ifndef SCENE_H
#define SCENE_H

#include "game_package.h"
#include "node.h"

namespace GameEngine
{
class Scene : public Node
{
public:
    Scene(std::string name);
    Scene(std::string name,int sceneID);
    virtual ~Scene();
    virtual void update(unsigned int elapsedTime) { Node::update(elapsedTime);}

    virtual void enter_scene();
    virtual void leave_scene();

    int get_scene_id();
    void set_scen_id(int sceneID);

    std::vector<Game_Package> &get_package();

    virtual void received_network_data(Game_Package *package);
protected:
    std::vector<Game_Package> _package;
private:
    int _scene_id;
};
}

#endif // SCENE_H
