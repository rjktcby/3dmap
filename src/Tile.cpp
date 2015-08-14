#include <iostream>
#include <cstdlib>

#include "Renderer.h"
#include "Tile.h"

// #define _TILE_DEBUG 1

Tile::Tile (tileId _id, glm::vec3 _pos, float _size)
        : id(_id), pos(_pos), size(_size)
{
        color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

        worldTopLeft = pos;
        worldTopRight = glm::vec3(pos[0]+size, pos[1], pos[2]);
        worldBottomRight = glm::vec3(pos[0]+size, pos[1], pos[2]+size);
        worldBottomLeft = glm::vec3(pos[0], pos[1], pos[2]+size);
}

void Tile::setTextureCoords(TexCoords tCoords)
{
        texCoords = tCoords;
}

void Tile::setColor(glm::vec4 c)
{
        color = c;
}

#define LEN(x1, y1, x2, y2) sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))

#define VISIBLE(p, v) ((p[0] >= v[0]) && (p[0] <= v[2]) && (p[1] >= v[1]) && (p[1] <= v[3]))

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a > b) ? (a) : (b))

float Tile::screenArea(Renderer *renderer)
{
        glm::vec2 screenTopLeft, screenTopRight, screenBottomLeft, screenBottomRight;

        renderer->worldToScreen(worldTopLeft, screenTopLeft);
        renderer->worldToScreen(worldTopRight, screenTopRight);
        renderer->worldToScreen(worldBottomLeft, screenBottomLeft);
        renderer->worldToScreen(worldBottomRight, screenBottomRight);

#ifdef _TILE_DEBUG
        std::cout << "[Tile] id=" << id << " screenCoords= "
                  << screenTopLeft << " -- " << screenBottomRight << std::endl;
#endif

        float xmin, xmax, ymin, ymax;
        xmin = min(screenTopLeft[0], min(screenTopRight[0], min(screenBottomRight[0], screenBottomLeft[0])));
        ymin = min(screenTopLeft[1], min(screenTopRight[1], min(screenBottomRight[1], screenBottomLeft[1])));
        xmax = max(screenTopLeft[0], max(screenTopRight[0], max(screenBottomRight[0], screenBottomLeft[0])));
        ymax = max(screenTopLeft[1], max(screenTopRight[1], max(screenBottomRight[1], screenBottomLeft[1])));

        glm::vec4 viewport = renderer->getViewport();

        if (xmax < viewport[0] || xmin > viewport[2] ||
            ymax < viewport[1] || ymin > viewport[3])
        {
                // Tile not visible
                return 0.0f;
        }

        float topSide = LEN(screenTopLeft[0], screenTopLeft[1], screenTopRight[0], screenTopRight[1]);
        float leftSide = LEN(screenTopLeft[0], screenTopLeft[1], screenBottomLeft[0], screenBottomLeft[1]);
        float bottomSide = LEN(screenBottomLeft[0], screenBottomLeft[1], screenBottomRight[0], screenBottomRight[1]);
        float rightSide = LEN(screenBottomRight[0], screenBottomRight[1], screenTopRight[0], screenTopRight[1]);

        float halfPer = (topSide + leftSide + bottomSide + rightSide) * 0.5f;
        float area = sqrt((halfPer-topSide)*(halfPer-leftSide)*(halfPer-bottomSide)*(halfPer-rightSide));

        return area;
}

void Tile::appendToBatch(struct TileVertex *batch)
{
        batch[0].x = pos[0];      batch[0].y = pos[1]; batch[0].z = pos[2];
        batch[1].x = pos[0]+size; batch[1].y = pos[1]; batch[1].z = pos[2];
        batch[2].x = pos[0]+size; batch[2].y = pos[1]; batch[2].z = pos[2]+size;
        batch[3].x = pos[0];      batch[3].y = pos[1]; batch[3].z = pos[2]+size;

        batch[0].tx = texCoords.coords[0];      batch[0].ty = texCoords.coords[1];
        batch[1].tx = texCoords.coords[2];      batch[1].ty = texCoords.coords[1];
        batch[2].tx = texCoords.coords[2];      batch[2].ty = texCoords.coords[3];
        batch[3].tx = texCoords.coords[0];      batch[3].ty = texCoords.coords[3];

        batch[0].cR = color[0]; batch[0].cG = color[1]; batch[0].cB = color[2];
        batch[1].cR = color[0]; batch[1].cG = color[1]; batch[1].cB = color[2];
        batch[2].cR = color[0]; batch[2].cG = color[1]; batch[2].cB = color[2];
        batch[3].cR = color[0]; batch[3].cG = color[1]; batch[3].cB = color[2];
}

tileId Tile::getId() const
{
        return id;
}

GLuint Tile::getTexId() const
{
        return texCoords.texId;
}