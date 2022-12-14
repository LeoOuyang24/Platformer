#include "physics.h"

#include "levelPhysics.h"
#include "player.h"
#include "SDLHelper.h"

PlayerMoveComponent::PlayerMoveComponent(Entity& entity) : BasicMoveComponent(glm::vec4(0,0,PLAYER_WIDTH,PLAYER_HEIGHT),entity), ComponentContainer<PlayerMoveComponent>(entity)
{

}

void PlayerMoveComponent::update()
{
    ForceVector gravity = Level::level.getGravity();
    setTilt(atan2(gravity.x,gravity.y)); //x and y are reversed on purpose. The math is more intuitive this way
    glm::vec2 moveDir = PLAYER_SPEED*(glm::vec2(gravity.y,-1*gravity.x));
    if (KeyManager::isPressed(SDLK_LEFT))
    {
        addMoveVec(-1.0f*moveDir);
    }
    if (KeyManager::isPressed(SDLK_RIGHT))
    {
        addMoveVec(1.0f*moveDir);
    }
    bool wasOnGround = onGround; //if we were previously on the ground; helps us stick to the ground when going down slopes
    setRect(Level::level.terrain.getPathEnd(rect,rect+glm::vec4(moveVec,0,0)));
    //std::cout << rect.y << "\n";
    onGround = Level::level.terrain.onGround(rect,gravity);
    //std::cout << wasOnGround << " " << onGround <<"\n";
    if ((wasOnGround ) || onGround)
    {
        float a = Level::level.terrain.getHeight({rect.x,rect.y + rect.a + 1},gravity); //find the highest level we should be at that isn't too high
        a = a < rect.y + Level::level.terrain.space/2 ? rect.y : a - rect.a - 1;
        float b = Level::level.terrain.getHeight({rect.x + rect.z, rect.y + rect.a + 1},gravity);
        b = b < rect.y + Level::level.terrain.space/2 ? rect.y : b - rect.a - 1;
        rect.y = std::min(a,b);
        if (KeyManager::getJustPressed() == SDLK_SPACE )
        {
            if (ForcesComponent* forces = entity->getComponent<ForcesComponent>())
            {
                forces->addForce(-1.0f*(gravity));
            }
        }
        PolyRender::requestRect(rect,glm::vec4(1,0,0,1),0,0,1);
    }


    moveVec = glm::vec2(0);
    //BasicMoveComponent::update();
}

bool PlayerMoveComponent::isOnGround()
{
    return onGround;
}
