#include "levelPhysics.h"
#include "player.h"

LevelPhysicsComponent::LevelPhysicsComponent(Entity& entity) : ForcesComponent(entity), ComponentContainer<LevelPhysicsComponent>(entity)
{

}
void LevelPhysicsComponent::update()
{
    if (move)
    {
        if (!entity->getComponent<PlayerMoveComponent>()->isOnGround())
        {
            gravityComponent += Level::level.gravity;
            addForce(Level::level.gravity);
        }
        else
        {
            gravityComponent = glm::vec2(0);
            finalForce.y = std::min(0.0f,finalForce.y);
        }
        //move->addMoveVec(gravityComponent);
    }
    ForcesComponent::update();
}

Level Level::level;
