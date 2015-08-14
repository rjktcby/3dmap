#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <list>

#include "Tile.h"
#include "glm/glm.hpp"

class Dispatcher;

class Renderer
{
        Dispatcher *dispatcher;

        glm::vec3 translation, rotation;
        glm::mat4 mViewMatrix, mModelMatrix, mModelViewMatrix, mProjMatrix, mMVPMatrix, mInvMVPMatrix;
        glm::vec4 mViewport;

        std::list<Tile *> tiles;

        float zoomLevel, zoomFactor;

        void updateModelViewMatrix();
        void updateProjectionMatrix();
        void updateMVPMatrix();

        void removeTiles();
        void tryAddTile(tileId id, glm::vec3 pos, float tile_size);
        void callBatch(struct TileVertex *vertices, int nTiles, GLuint texId);

public:
        Renderer(Dispatcher *d, int width, int height);
        ~Renderer();

        void reshape(int width, int height);
        glm::vec4 getViewport() const;
        void draw();
        void rotate(glm::vec3 diff);
        void translate(glm::vec2 from, glm::vec2 to);
        void zoomUp();
        void zoomDown();

        void updateTiles();

        void screenToWorld(glm::vec2 screen, glm::vec3 &world);
        void worldToScreen(glm::vec3 world, glm::vec2 &screen);
};

#endif //RENDERER_H_INCLUDED