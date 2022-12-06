#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "components.h"

class PlayerMoveComponent : public BasicMoveComponent, public ComponentContainer<PlayerMoveComponent>
{
    constexpr static int PLAYER_WIDTH = 10;
    constexpr static int PLAYER_HEIGHT = 20;
    constexpr static float PLAYER_SPEED = .05;
    bool onGround = false;
public:
    PlayerMoveComponent(Entity& entity);
    void update();
    bool isOnGround();
};

#endif // PLAYER_H_INCLUDED
