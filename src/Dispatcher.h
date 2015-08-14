#ifndef DISPATCHER_H_INCLUDED
#define DISPATCHER_H_INCLUDED

#include "Renderer.h"
#include "Fetcher.h"
#include "Atlas.h"
#include "buffer.hpp"
#include "tileId.hpp"
#include "glm/glm.hpp"
#include <vector>

class Dispatcher {

        Renderer *renderer;
        Fetcher *fetcher;
        Atlas *atlas;

        bool tileUpdateNeeded;

public:

        Dispatcher(unsigned int width, unsigned int height);
        ~Dispatcher();

        // Renderer-related
        void windowSizeChanged(unsigned int width, unsigned int height);
        void rotateScene(glm::vec3 diff);
        void translateScene(glm::vec2 from, glm::vec2 to);
        void zoomSceneUp();
        void zoomSceneDown();
        void renderScene();    //Here we render as soon as possible
        void idle();           //Here we do all the rest

        // Fetcher-related
        void addTileFetchRequest(tileId id);
        void tileRequestFinished(tileId id, buffer *buf);

        // Atlas-related
        TexCoords getTileTexCoords(tileId id);
        TexCoords getParentTileTexCoords(tileId id);
        void removeTileFromAtlas(tileId id);
        void markAllTilesUnused();
        void removeUnusedTiles();
};

#endif //DISPATCHER_H_INCLUDED