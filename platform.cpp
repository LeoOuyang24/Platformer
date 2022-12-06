#include "render.h"

#include "platforms.h"



void MeshTerrain::MeshTerrainRenderInfo::render()
{
        if (triangles.size() > 0)
        {

            glDisable(GL_PRIMITIVE_RESTART);
            if (RenderCamera::currentCamera)
            {
                auto rect = RenderCamera::currentCamera->getRect();
                auto cameraPos = glm::vec3(rect.x,rect.y,0);
                auto project = glm::lookAt(cameraPos - glm::vec3(0,0,-3),cameraPos, glm::vec3(0,1,0)); //this code positions the camera 3 units away from the screen and looks at the center of the screen

                PolyRender::polyRenderer.setXRange(rect.x,rect.x + rect.z);
                PolyRender::polyRenderer.setYRange(rect.y, rect.y + rect.a);
                PolyRender::polyRenderer.use(value_ptr(project));
            }
            else
            {
                PolyRender::polyRenderer.use();
            }
            if (!drawn)
            {
                glBindVertexArray(VAO);

                glBindBuffer(GL_ARRAY_BUFFER,vertexVBO);
                glBufferData(GL_ARRAY_BUFFER,triangles.size()*sizeof(GLfloat),&triangles[0],GL_STATIC_DRAW);
                glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
                glEnableVertexAttribArray(0);
                //glVertexAttribDivisor(0,1);

                glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
                glBufferData(GL_ARRAY_BUFFER,colors.size()*sizeof(GLfloat), &colors[0], GL_STATIC_DRAW);
                glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
                glEnableVertexAttribArray(1);
                //glVertexAttribDivisor(1,1);

                glDrawArrays(GL_TRIANGLES,0,triangles.size()/3);

                glBindBuffer(GL_ARRAY_BUFFER,0);
                glBindVertexArray(0);

                drawn = true;
            }
            else
            {
                glBindVertexArray(VAO);
                PolyRender::polyRenderer.use();
                glDrawArrays(GL_TRIANGLES,0,triangles.size()/3);
                glBindVertexArray(0);
            }
            glEnable(GL_PRIMITIVE_RESTART);
            //renderMesh();
        }

}

void MeshTerrain::MeshTerrainRenderInfo::addRect(const glm::vec4& rect, PixelData data, int channels, int width)
{
    triangles.push_back(rect.x);
    triangles.push_back(rect.y);
    triangles.push_back(0);

    triangles.push_back(rect.x + rect.z);
    triangles.push_back(rect.y);
    triangles.push_back(0);

    triangles.push_back(rect.x);
    triangles.push_back(rect.y + rect.a);
    triangles.push_back(0);

    triangles.push_back(rect.x + rect.z);
    triangles.push_back(rect.y);
    triangles.push_back(0);

    triangles.push_back(rect.x);
    triangles.push_back(rect.y + rect.a);
    triangles.push_back(0);

    triangles.push_back(rect.x + rect.z);
    triangles.push_back(rect.y + rect.a);
    triangles.push_back(0);
    int modTopLeft = rect.x*channels + rect.y*channels*width ; //top left corner of pixel data
    for (int j = 0; j < 6; ++j)
    {
        int newJ = j - 2*(j > 2); //modified iterator to ensure we render the right corners.
        int colorTopLeft = modTopLeft + channels*((rect.z - 1)*(newJ%2) + (rect.a - 1)*(newJ/2)*(width));
        colors.push_back(data[colorTopLeft]/255.0);
        colors.push_back(data[colorTopLeft + 1]/255.0);
        colors.push_back(data[colorTopLeft + 2]/255.0);
        colors.push_back(1);
    }
}

void MeshTerrain::MeshTerrainRenderInfo::addTriangle(MeshTerrainTiles tile, const glm::vec2& tileTopLeft, PixelData data, int channels, int width, int space)
{
            triangles.push_back(tileTopLeft.x + (tile/2)*(space));
            triangles.push_back(tileTopLeft.y );
            triangles.push_back(1);

            triangles.push_back(tileTopLeft.x + (tile/2)*(space));
            triangles.push_back(tileTopLeft.y + space );
            triangles.push_back(1);

            triangles.push_back(tileTopLeft.x + (1 - tile/2)*(space));
            triangles.push_back(tileTopLeft.y + (tile%2)*(space));
            triangles.push_back(1);

            int topLeftIndex = tileTopLeft.x * channels + tileTopLeft.y*width*channels;
            for (int i = 0; i < 3; ++i)
            {
                int dataIndex =  topLeftIndex + channels*(space - 1)*((tile/2 + (i == 2))%2 + (i%2 + (i == 2)*tile%2)*width);
                colors.push_back(data[dataIndex]/255.0); //find the color at the corresponding indice and push it back for rendering
                colors.push_back(data[dataIndex + 1]/255.0);
                colors.push_back(data[dataIndex + 2]/255.0);
                colors.push_back(1);
            }
}

void MeshTerrain::setup(std::string source)
{
    int width, height, channels;
    PixelData data = stbi_load(source.c_str(),&width, &height, &channels, 0);
    if (!data)
    {
        std::cout << "Error loading: " << source << "\n";
        return;
    }
    glGenVertexArrays(1,&renderInfo.VAO);
    glGenBuffers(1,&renderInfo.vertexVBO);
    glGenBuffers(1,&renderInfo.colorVBO);
    region = {0,0,width,height};
    horizTiles = width/space;
    vertTiles = height/space;
    tiles.reserve(getArea());

    auto helper = [this,width,height,&data,channels](int tileIndex){ //index of the tile we are processing
        //there's a difference between the index as in tile we are on, and the index as in data, which is the pixel data. We'll refer to the latter as data index and the former as color index

        auto tileIndexToColorIndex = [this,width,channels](int tileIndex){
        if (horizTiles == 0)
        {
            return 0;
        }
        return (tileIndex%horizTiles + tileIndex/horizTiles*width)*space*channels + channels - 1;
        }; //useful lambda for converting tileIndex to colorIndex

        int modTopLeft = tileIndexToColorIndex(tileIndex); //convert tile index to color index
        bool topLeft = data[modTopLeft]; //data index of the transparency 4 tiles
        bool topRight = data[modTopLeft + (space-1)*channels];
        bool botLeft = data[modTopLeft + (space-1)*channels*width];
        bool botRight = data[modTopLeft + (space-1)*channels*(width + 1)];
        int tileInt = 0;
        MeshTerrainTiles tile;
        if (topLeft && topRight && botLeft && botRight) //if all 4 corners are solid, solid tile
        {
            tile = FULL;
        }
        else
        {
            tileInt = ((topLeft && topRight) + 2*(botLeft && botRight)); //these insane lines of code uses base 3 to store info about the 4 corners. Least signigificant bit is 1 if top line exists, 2 if bot line exists (they can't both exist otherwise it's a full tile)
            tileInt += (3*( (topLeft && botLeft) + 2*(topRight && botRight))); //most significant bit is 1 if left line is solid, 2 if right line is solid
                                                                        //Example: 12 would mean solid left line, solid bottom line.
            if (tileInt/3 == 0 || tileInt % 3 == 0)                           //If there is no vertical line or no bottom line, then the tile is empty
            {
                tile = EMPTY;
            }
            else                                                        //otherwise, convert to base 2, which conveniently lines up with the MeshTerrainTiles enums
            {
                tile = MeshTerrainTiles(2*(tileInt / 3 - 1) + (tileInt % 3 - 1));               //example: 12 -> 2*(1 - 1)+ (2 - 1) = 01 = BOTLEFT
            }
        }
        //std::cout << (counter >> 4) << "\n";
         if (tile == FULL) //4 points
        {
            renderInfo.addRect(glm::vec4(indexToPoint(tileIndex),space,space),data,channels,width);
        }
        else if (tile != EMPTY) //3 points
        {
            renderInfo.addTriangle(tile,indexToPoint(tileIndex),data,channels,width,space);
        }
        tiles.push_back(tile);
    };
    for (int i = 0; i < (horizTiles)*(vertTiles); ++i)
    {
        helper(i);
    }
}

void MeshTerrain::update()
{
    if (KeyManager::isPressed(SDLK_BACKQUOTE))
    {
        for (int i = region.x; i < region.x + region.z; i+= space)
        {
            PolyRender::requestLine(glm::vec4(i,region.y,i,region.y + region.a),glm::vec4(1,0,0,1),1,1);
        }
        for (int j = region.y; j < region.y + region.a; j += space)
        {
            PolyRender::requestLine(glm::vec4(region.x,j,region.x + region.z,j),glm::vec4(1,0,0,1),1,1);
        }
    }
    renderInfo.render();
}

glm::vec2 MeshTerrain::getPathEnd(const glm::vec2& start, const glm::vec2& end)
{   //given a vector going from start to end, returns the furthest end point that start can go to
    glm::vec2 finalEnd = end; //our final result
    MeshTerrainTiles tile = tiles[pointToIndex(end)];
    glm::vec4 tileRect = glm::vec4(normalizePoint(end),space,space); //dimensions and topLeft of the tile
    switch (tile)
    {
    case EMPTY:
        //if the tile is empty, "end" should be reachable
        break;
    case FULL:
        {
            if (FLOAT_COMPARE(start.y,tileRect.y + tileRect.a,3,<) && FLOAT_COMPARE(start.y,tileRect.y,3,>)) //line intersects with one of the left or right sides of the tile
            {
                finalEnd.x = tileRect.x + (tileRect.z)*(start.x > end.x) +  (start.x > end.x)*2 - 1; //the << 1 - 1 moves our x 1 unit to the left or right depending on if we are moving to the right or left
                finalEnd.y = start.y + (finalEnd.x - start.x)/(end.x - start.x)*(end.y - start.y); //don't gotta worry abou end.x == start.x because it would be impossible for a vertical line to intersect with the sides of a rectangle
            }
            else //otherwise, we intersect with top or bottom of tile
            {
                finalEnd.y = tileRect.y + (tileRect.a)*(start.y > end.y) + ((start.y > end.y)*2 - 1);
                finalEnd.x = start.x + (finalEnd.y - start.y)/(end.y - start.y)*(end.x - start.x);
            }
            break;
        }
    case TOPLEFT:
    case TOPRIGHT:
    case BOTRIGHT:
    case BOTLEFT:
        //for now, we only consider the slant, not the sides
        bool posSlope = ((tile%2) == (tile >>2)); //the slope is positive if botSide and rightSide both exist or both dont exist
        glm::vec3 point = lineLineIntersect(start, end,
                                            glm::vec2(tileRect.x,tileRect.y + tileRect.a*posSlope),
                                            glm::vec2(tileRect.x + tileRect.z,tileRect.y + tileRect.a*(!posSlope)));
        if (point.z)
        {
            finalEnd = {point.x,point.y};
        }
        break;
    }
    return finalEnd;
}

glm::vec4 MeshTerrain::getPathEnd(const glm::vec4& start, const glm::vec4& end)
{
    //assumes start and end have the same dimensions

    bool goingRight = end.x > start.x;
    bool goingDown = end.y > start.y;

    float x,y;
    if (goingRight)
    {
        x = std::min(getPathEnd({start.x + start.z,start.y},{end.x + end.z,start.y}).x,
                    getPathEnd({start.x + start.z, start.y + start.a},{end.x + end.z, start.y + start.a}).x) - start.z;
    }
    else
    {
        x = std::max(getPathEnd({start.x,start.y},{end.x,start.y}).x,
                    getPathEnd({start.x , start.y + start.a},{end.x, start.y + start.a}).x);
    }
    if (goingDown)
    {
        y = std::min(getPathEnd({start.x, start.y + start.a},{start.x, end.y + end.a}).y,
                     getPathEnd({start.x + start.z, start.y + start.a},{start.x + start.z, end.y + end.a}).y) - start.a;
    }
    else
    {
        y = std::max(getPathEnd({start.x,start.y},{start.x,end.y}).y,
                    getPathEnd({start.x +start.z, start.y},{start.x + start.z, end.y}).y);
    }
    //printRect(glm::vec4(start.x,start.y,x,y));
    return glm::vec4(x,y,end.z,end.a);
}

float MeshTerrain::getHeight(const glm::vec2& point)
{
    float height = 0;
    int index = pointToIndex(point);
    MeshTerrainTiles tile = tiles[index];

    glm::vec4 tileRect = glm::vec4(normalizePoint(point),space,space); //dimensions and topLeft of the tile
    //PolyRender::requestRect(tileRect,glm::vec4(1,0,0,1),0,0,1);
    switch (tile)
    {
    case EMPTY: //if tile is empty, we don't do anything unless there is a non-empty tile directly below it, in which case we use the height of tha ttile
        {
            glm::vec2 below = {point.x,point.y + space}; //the point in the tile above us
            if (index >= vertTiles*horizTiles || tiles[pointToIndex(below)] == EMPTY) //if there are no tile below or it's empty ...
            {
                return point.y; //... don't change y
            }
            else
            {
                return getHeight(below);
            }
             break;
        }
    case FULL:
        {
            glm::vec2 above = {point.x,point.y - space}; //the point in the tile above us
            if (index < horizTiles || tiles[pointToIndex(above)] == EMPTY) //if there are no tile above or it's empty ...
            {
                height = 0; //... our y is the y of the tile
            }
            else
            {
                return getHeight(above);
            } //if tile is full, check the tile above it if it exists; 0 otherwise
             break;
        }
    case TOPLEFT:
    case TOPRIGHT:
        height = 0;
        break;
    case BOTRIGHT:
        height = space - (point.x - tileRect.x);
        break;
    case BOTLEFT:
        height = point.x - tileRect.x;
        break;
    }
    return tileRect.y + height;
}

bool MeshTerrain::onGround(const glm::vec4& rect)
{
    return inWall(glm::vec2(rect.x,rect.y + rect.a + 1),glm::vec2(rect.x + rect.z, rect.y + rect.a + 1));
}

bool MeshTerrain::inWall(const glm::vec2& point)
{
    if (tiles.size() == 0 || !pointInVec(region,point)) //if no terrain or if point is outside our region, point can't be in wall
    {
        return false;
    }
    int index = pointToIndex(point); //top left index

    if (tiles[index] == FULL)
    {
        return true;
    }
    else if (tiles[index] != EMPTY)
    {
        /*if (positive) //branching version. Same as the uncommented code but a lot more readable
        {
            if (above)
            {
                return point.y <= point.x;
            }
            else
            {
                return point.y >= point.x;
            }
        }
        else
        {
            if (above)
            {
                return point.y <= -1*point.x + space;
            }
            else
            {
                return point.y >= -1*point.x + space;
            }
        }*/
        int bigger = (tiles[index]%2 == 1)*2 - 1; //1 if there is a bottom wall (and thus to be inside the wall y would have to be bigger than x), -1 otherwise
        bool positive = (tiles[index]%2 != tiles[index]/2); //true if slope is positive
        glm::vec2 normalized = normalizePoint(point); //top left corner of the tile (also our local (0,0))
        return FLOAT_COMPARE(bigger*(point.y - normalized.y),bigger*((positive*2 - 1)*(point.x - normalized.x) + !positive*space),.001,>);   //check if point is in the wall by seeing if it is below a line of slope +- 1;
                                                                    //if bigger is 1, point.y must be greater than point.x to be in the wall. Otherwise, point.y must be less
                                                                    //if positive is false, then the y intercept is "space", other wise it's 0.
    }
    return false;
}
bool MeshTerrain::inWall(const glm::vec2& p1, const glm::vec2& p2) //returns true if the line is in a wall. Not perfectly accurate
{
    if (tiles.size() == 0  || !pointInVec(region,p1,0) || !pointInVec(region,p2))
    {
        return false;
    }
    glm::vec2 slope = betterNormalize(p2 - p1);
    glm::vec2 start = p1; //first point we will consider

    while (true) //process every point along the line every "space" intervals, where "space" is the space between tiles
    {
        if (inWall(start)) //if we found a point that collides with a wall, return true
        {
                return true;
        }
        if (start == p2 || glm::distance(start,p2) <= .0001 || !pointInVec(region,start)) //reached the final point and didn't collide with a wall or went out of bounds, so we are done
        {
            return false;
        }
        if (glm::distance(start,p2) <= .0001) //if we are already very close to "p2", clamp onto "p2". Setting "start" = "p2" prevents floating point errors
        {
            start = p2;
        }
        else  //otherwise, move onto the next point that is "space" away or clamp onto "p2"
        {
            start += std::min((float)space,glm::distance(start,p2))*slope;
        }
    }
    return false; //no tiles collide with wall, no wall collision (never runs)
}

bool MeshTerrain::inWall(const glm::vec4& rect) //tests if all 4 corners of the rectangle are in a wall. Not perfectly accurate, but works for small rectangles
{
    return inWall({rect.x,rect.y}) || inWall({rect.x + rect.z,rect.y}) || inWall({rect.x, rect.y + rect.a}) || inWall({rect.x + rect.z, rect.y + rect.a});
}
