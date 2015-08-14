#include "Atlas.h"

#include <iostream>
#include <algorithm>

#include <GLFW/glfw3.h>

// #define _ATLAS_DEBUG 1

Atlas::Atlas(unsigned int w, unsigned int h, unsigned int tileW, unsigned int tileH, unsigned int n)
{
        // Generate the OpenGL texture object
        format = GL_RGBA;
        width = w;
        height = h;
        tileWidth = tileW;
        tileHeight = tileH;
        nAtlases = n;

        for (int i = 0; i < nAtlases; i++) {
                GLuint texture;

                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, (void *)NULL);

                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                textureIds.push_back(texture);
        }

        tilesInRow = w/tileWidth;
        tilesInCol = h/tileHeight;

        tilesInAtlas = tilesInRow*tilesInCol;
        totalTiles = tilesInAtlas * nAtlases;
        tilesInPositions.resize(totalTiles);
        usedTiles.resize(totalTiles);
        usedTiles.assign(totalTiles, false);

        for (int i = 0; i < totalTiles; i++) {
                tilesInPositions.push_back(tileId(0,0,0));
                freePositions.push_back(i);
        }
}

Atlas::~Atlas()
{
        //TODO
        return;
}

TexCoords Atlas::getTexCoordsByPos(int pos)
{
        int atlasNum = pos/tilesInAtlas;
        int posInAtlas = pos%tilesInAtlas;

        int rowNum = posInAtlas/tilesInRow;
        int colNum = posInAtlas%tilesInRow;

        glm::vec4 coords = glm::vec4((double)colNum*tileWidth/width, (double)rowNum*tileHeight/height,
                (double)(colNum+1)*tileWidth/width, (double)(rowNum+1)*tileHeight/height);

        return TexCoords(textureIds[atlasNum], coords);
}

void Atlas::addImgForTile(tileId id, buffer *buf, ImgInfo imgInfo)
{
        if (imgInfo.format != format) {
                std::cout << "[Atlas] error: wrong texture format=" << imgInfo.format << " want=" << format << std::endl;
                return;
        }

        if (freePositions.size() == 0) {
                std::cout << "[Atlas] error: can't fit image into atlas";
                return;
        }

        int pos = freePositions.front();
        int atlasNum = pos/tilesInAtlas;
        int posInAtlas = pos%tilesInAtlas;
        int rowNum = posInAtlas/tilesInRow;
        int colNum = posInAtlas%tilesInRow;
        freePositions.pop_front();
        tilesInPositions[pos] = id;

#ifdef _ATLAS_DEBUG
        std::cout << "[Atlas] texture coords for tile " << id
                  << ": atlas=" << atlasNum << " pos="
                  << glm::vec2(rowNum, colNum) << std::endl;
#endif

        glBindTexture(GL_TEXTURE_2D, textureIds[atlasNum]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, colNum*tileWidth, rowNum*tileHeight, imgInfo.width, imgInfo.height, format, GL_UNSIGNED_BYTE, buf->data());
}

TexCoords Atlas::getParentTileTexCoords(tileId id)
{
        tileId parentId = tileId(id.z()-1, id.x()/2, id.y()/2);
        if (parentId.z() == 0)
                return TexCoords(0, glm::vec4(0.0f));
        TexCoords parentCoords = getTileTexCoords(parentId);
        if (parentCoords.texId == 0)
                parentCoords = getParentTileTexCoords(parentId);
        if (parentCoords.texId == 0)
                return TexCoords(0, glm::vec4(0.0f));

        glm::vec4 pCoords = parentCoords.coords;
        glm::vec4 childCoords(0);
        childCoords[0] = pCoords[0]+(pCoords[2]-pCoords[0])*0.5*(id.x()-parentId.x()*2);
        childCoords[1] = pCoords[1]+(pCoords[3]-pCoords[1])*0.5*(id.y()-parentId.y()*2);
        childCoords[2] = childCoords[0] + (pCoords[2]-pCoords[0])*0.5;
        childCoords[3] = childCoords[1] + (pCoords[3]-pCoords[1])*0.5;


#ifdef _ATLAS_DEBUG
        std::cout << "[Atlas] Found parent tile=" << parentId << "for tile=" << id << std::endl;
        std::cout << "[Atlas] Parent coords=" << pCoords
                  << "Child coords=" << childCoords << std::endl;
#endif

        return TexCoords(parentCoords.texId, childCoords);
}

TexCoords Atlas::getTileTexCoords(tileId id)
{
        std::vector<tileId>::iterator found = std::find(tilesInPositions.begin(), tilesInPositions.end(), id);
        if (found == tilesInPositions.end()) {
                return TexCoords(0, glm::vec4(0.0f));
        }

        int pos = found - tilesInPositions.begin();
        usedTiles[pos] = true;

        return getTexCoordsByPos(pos);
}

void Atlas::removeTile(tileId id)
{
        std::vector<tileId>::iterator found = std::find(tilesInPositions.begin(), tilesInPositions.end(), id);
        if (found == tilesInPositions.end()) {
                // std::cout << "[Atlas] attempt to remove already removed tile" << std::endl;
                return;
        }

        int pos = found - tilesInPositions.begin();
        removeTileAtPos(pos);
}

void Atlas::removeTileAtPos(int pos)
{
        if (std::find(freePositions.begin(), freePositions.end(), pos) != freePositions.end()) {
                freePositions.push_back(pos);
        }
}

void Atlas::markAllUnused()
{
        usedTiles.assign(totalTiles, false);
}

void Atlas::removeUnused()
{
        std::vector<bool>::iterator i;
        for (i = usedTiles.begin(); i < usedTiles.end(); i++) {
                int pos = i - usedTiles.begin();
                if ((*i) == false)
                        removeTileAtPos(pos);
        }
}