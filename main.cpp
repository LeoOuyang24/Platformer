#include <iostream>
#include <time.h>
#include <SDL.h>

#include "render.h"
#include "SDLHelper.h"
#include "FreeTypeHelper.h"

#include "player.h"
#include "levelPhysics.h"

int main(int args, char* argsc[])
{
    //delete ptr;
    const int screenWidth = 640;
    const int screenHeight = 640;
    srand(time(NULL));
    SDL_Window* window = SDL_CreateWindow("Project",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    SDL_StopTextInput();
    SDL_GL_CreateContext(window);
    RenderProgram::init(screenWidth,screenHeight);
    Font::init(screenWidth, screenHeight);
    PolyRender::init(screenWidth,screenHeight);
    SDL_Event e;
    bool quit = false;
    glClearColor(1,1,1,1);
    bool eventsEmpty = true;
        //std::cout << tree.count() << std::endl;
    Level::level.terrain.setup("terrain/skatepark.png");

    Entity player;
    player.addComponent(*(new PlayerMoveComponent(player)));
    player.addComponent(*(new RectRenderComponent(player,glm::vec4(0,0,1,1))));
    player.addComponent(*(new LevelPhysicsComponent(player)));

    player.getComponent<RectComponent>()->setPos(glm::vec2(10,300));
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            eventsEmpty = false;
            KeyManager::update(e);
            MouseManager::update(e);
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        if (eventsEmpty)
        {
            KeyManager::update(e);
            MouseManager::update(e);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        PolyRender::requestCircle(glm::vec4(Level::level.terrain.inWall(pairtoVec(MouseManager::getMousePos())),0,0,1),pairtoVec(MouseManager::getMousePos()),2,true,3);

        Level::level.terrain.update();
        player.update();

        SpriteManager::render();
        PolyRender::render();
       // Font::alef.write(Font::wordProgram,"asdf",320,320,0,1,{0,0,0});
        SDL_GL_SwapWindow(window);
        DeltaTime::update();
        eventsEmpty = true;
        //std::cout << DeltaTime::deltaTime << std::endl;
    }
    return 0;
}
