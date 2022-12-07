#ifndef PLATFORMS_H_INCLUDED
#define PLATFORMS_H_INCLUDED

#include "render.h"
#include "glGame.h"

#include "stb_image.h"

/*
    Represents terrain use a series of true falses
*/

struct MeshTerrain
{
    int space = 10; //space between tiles
    enum MeshTerrainTiles //the types of tiles we can have
    {
        //there are 4 types of triangle tiles, which are defined here by which sides of the tile are solid.
        //the way the first 4 enums are set up is that most sig bit = solid vert side (0 = left, 1 = right) and the least sig bit = solid horiz side (0 = top, 1 = bottom)
        TOPLEFT, //triangle with left and top side as legs = 00
        BOTLEFT, //triangle with left and bot side as legs = 01
        TOPRIGHT, //triangle with right and top side as legs = 10
        BOTRIGHT, //triangle with right and bottom side as legs = 11
        EMPTY, //completely empty
        FULL //completely full, square tile
    };
    typedef std::vector<MeshTerrainTiles> TileStorage;
    typedef unsigned char* PixelData; //used to define reading data in from an image

    struct MeshTerrainRenderInfo
    {
        //handles info to pass to OpenGL for rendering. This struct mainly exists for organization purposes
        std::vector<GLfloat> triangles; //vector of the tiles that form the triangles
        std::vector<GLfloat> colors;
        unsigned int VAO;
        unsigned int vertexVBO;
        unsigned int colorVBO;
        void render();
        bool drawn = false;
        //void addTile(MeshTerrainTiles tile, int pixelIndex, PixelData data); //add a tile to render. Must pass in the pixel data as well as the topleft index of the pixel
        void addRect(const glm::vec4& rect, PixelData data, int channels, int width); //add a rectangle to render at the provided coordinates. Must pass in pixel data, number of color channels, and the width of the image
        void addTriangle(MeshTerrainTiles tile, const glm::vec2& tileTopLeft, PixelData data, int channels, int width, int space); //adds a triangular tile. Must provide data, channels, tile size, as well as the index of the tile in the pixel data
    };


    MeshTerrainRenderInfo renderInfo;
    TileStorage tiles; //unique vector of tiles that are collidable
    glm::vec4 region;
    int horizTiles = 0,vertTiles = 0; //number of tiles in each dimension

    int getTileSize()
    {
        return tiles.size();
    }
    int getArea() //return number of tiles that fit in the grid (not the actual amount of tiles)
    {
        return (horizTiles)*(vertTiles);
    }
    void setup(std::string source);
    void update();
    int pointToIndex(const glm::vec2& point)
    {
        return ((int)point.x)/space + ((int)point.y)/space*(horizTiles);
    }
    glm::vec2 indexToPoint(int index)
    {
        return {index%(horizTiles)*space,index/(horizTiles)*space};
    }
    glm::vec2 normalizePoint(const glm::vec2& point)
    {
        return {floor(point.x/space)*space, floor(point.y/space)*space};
    }
    template<typename Func>
    auto processPoint(const glm::vec2& point, Func f) //given a point, find the 4 verticies  that contain the point and run "f" on the 4 verticies
    {
        //WARNING: Unsafe if point is in the mesh. Will crash
        //point is a point in the real world
        //f = ( const glm::vec2&, int, glm::vec2[]) -> auto
        //point to process, number of wall verticies, array of wall verticies
        int index = pointToIndex(point);
        int sum = tiles[index] + tiles[index + 1] + tiles[index + horizTiles + 1] + tiles[index + horizTiles + 1 + 1]; //number of tiles that are actual walls
        glm::vec2* verticies = new glm::vec2[sum];
        int ind = 0;
        for (int i = 0; i < 4; ++i)
        {
            int newIndex = index + i%2 + i/2*(horizTiles + 1);
            if (tiles[newIndex])
            {
                verticies[ind] = indexToPoint(newIndex);
                ind++;
            }
        }
        if constexpr(std::is_void<Func()>::value)
        {
            f(point, sum, verticies);
            delete[] verticies;
        }
        else
        {
            auto result = f(point,sum,verticies);
            delete[] verticies;
            return result;
        }
    }
    glm::vec2 getPathEnd(const glm::vec2& start, const glm::vec2& end);
    glm::vec4 getPathEnd(const glm::vec4& start, const glm::vec4& end);
    float getHeight(const glm::vec2& point, const glm::vec2& gravity = {0,1}); //get the highest y at the corresponding tile
    bool onGround(const glm::vec4& rect, const glm::vec2& gravity = {0,1}); //determines if a rect is on the ground based on the direction of gravity
    bool inWall(const glm::vec2& p1, const glm::vec2& p2);
    bool inWall(const glm::vec2& p1);
    bool inWall(const glm::vec4& rect);
    void renderMesh(const glm::vec4& wall = {1,0,0,1}, const glm::vec4& none = {0,0,0,1}) //renders a small circle at each point with color "wall" if the point is true, "none" if the point is none
    {
        int tilesSize = getTileSize();
        for (int i = 0; i < tilesSize; ++i)
        {
            PolyRender::requestNGon(10,indexToPoint(i),1,(tiles[i] ? wall : none),0,true,0);
        }
    }
};

#endif // PLATFORMS_H_INCLUDED
