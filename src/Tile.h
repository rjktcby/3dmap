#ifndef TILE_H_INCLUDED
#define TILE_H_INCLUDED

#include "types.h"
#include "glm/glm.hpp"
#include "tileId.hpp"


struct TileVertex
{
    float x, y, z;        // Vertex
    float cR, cG, cB;     // Color
    float tx, ty;         // Texture coords
};

class Renderer;

class Tile
{
        tileId id;
        glm::vec3 pos, worldTopLeft, worldTopRight, worldBottomLeft, worldBottomRight;
        glm::vec4 color;
        TexCoords texCoords;
        float size;

public:
        Tile (tileId id, glm::vec3 pos, float size);

        tileId getId() const;
        GLuint getTexId() const;

        void setTextureCoords(TexCoords tCoords);
        void appendToBatch(struct TileVertex *batch);
        void setColor(glm::vec4 color);
        float screenArea(Renderer *renderer);

};

#endif //TILE_H_INCLUDED