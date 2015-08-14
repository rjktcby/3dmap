#ifndef ATLAS_H_INCLUDED
#define ATLAS_H_INCLUDED

#include "tileId.hpp"
#include <vector>
#include <deque>

#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "buffer.hpp"
#include "tileId.hpp"
#include "types.h"

class Atlas
{
        unsigned int width, height;
        unsigned int tileWidth, tileHeight;
        int nAtlases;
        int tilesInCol, tilesInRow, tilesInAtlas, totalTiles;

        std::vector<GLuint> textureIds;
        GLint format;

        std::deque<int> freePositions;
        std::vector<bool> usedTiles;
        std::vector<tileId> tilesInPositions;

        TexCoords getTexCoordsByPos(int pos);
        void removeTileAtPos(int pos);
public:

        Atlas(unsigned int w, unsigned int h, unsigned int tw, unsigned int th, unsigned int count);
        ~Atlas();
        void addImgForTile(tileId id, buffer *buf, ImgInfo imgInfo);
        void removeTile(tileId id);
        TexCoords getTileTexCoords(tileId id);
        TexCoords getParentTileTexCoords(tileId id);

        void markAllUnused();
        void removeUnused();
};

#endif //ATLAS_H_INCLUDED
