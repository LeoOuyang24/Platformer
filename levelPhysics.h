#ifndef LEVELPHYSICS_H_INCLUDED
#define LEVELPHYSICS_H_INCLUDED

#include "platforms.h"
#include "physics.h"

class LevelPhysicsComponent : public ForcesComponent, public ComponentContainer<LevelPhysicsComponent>
{
    ForceVector gravityComponent = glm::vec2(0);
public:
    LevelPhysicsComponent(Entity& entity);
    void update();
};

struct Level
{
    MeshTerrain terrain;
    ForceVector gravity = glm::vec2(0,.001f);
    static Level level;
};

#endif // LEVELPHYSICS_H_INCLUDED
