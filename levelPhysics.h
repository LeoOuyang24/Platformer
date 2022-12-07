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
    ForceVector gravity = glm::vec2(0,1); //gravity is the unit vector for the direction of gravity. A lot of things rely only on the direction so this might make it easier than calling normalize a bunch (which requires square roots)
    float gravMag = .001f;
    static Level level;
    ForceVector getGravity();
};

#endif // LEVELPHYSICS_H_INCLUDED
