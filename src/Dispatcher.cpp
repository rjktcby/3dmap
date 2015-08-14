#include "Dispatcher.h"
#include "PngDecoder.h"
#include <iostream>

// #define _DISPATCHER_DEBUG 1

Dispatcher::Dispatcher(unsigned int width, unsigned int height)
{
        atlas = new Atlas(2048, 2048, 256, 256, 8);

        fetcher = new Fetcher(this);
        renderer = new Renderer(this, width, height);

        tileUpdateNeeded = true;
}

Dispatcher::~Dispatcher()
{
        delete renderer;
        delete fetcher;
}

void Dispatcher::windowSizeChanged(unsigned int newWidth, unsigned int newHeight)
{
        renderer->reshape(newWidth, newHeight);

        tileUpdateNeeded = true;
}

void Dispatcher::rotateScene(glm::vec3 diff)
{
        renderer->rotate(diff);

        tileUpdateNeeded = true;
}

void Dispatcher::translateScene(glm::vec2 from, glm::vec2 to)
{
        renderer->translate(from, to);

        tileUpdateNeeded = true;
}

void Dispatcher::zoomSceneUp()
{
        renderer->zoomUp();

        tileUpdateNeeded = true;
}

void Dispatcher::zoomSceneDown()
{
        renderer->zoomDown();

        tileUpdateNeeded = true;
}

void Dispatcher::renderScene()
{
        if (tileUpdateNeeded) {
                renderer->updateTiles();
                tileUpdateNeeded = false;
        }
        renderer->draw();
}

void Dispatcher::idle()
{
        fetcher->perform();
}

void Dispatcher::addTileFetchRequest(tileId id)
{
        fetcher->addRequest(id);
}

void Dispatcher::tileRequestFinished(tileId id, buffer *buf)
{
#ifdef _DISPATCHER_DEBUG
        std::cout << "[Dispatcher] Tile " << id << "fetched (" << buf->size() << " bytes)" << std::endl;
#endif //_DISPATCHER_DEBUG

        PngDecoder *decoder = new PngDecoder(buf);

        int rc = 0;
        if (0 != (rc = decoder->validate())) {
                std::cout << "[Dispatcher] error: unable to decode image for tile (" << id.z() << "," << id.x() << "," << id.y() << ")" << std::endl;
                return;
        }

        buffer *imgData = new buffer();

        ImgInfo imgInfo = decoder->getImgInfo();

        decoder->decodeIntoBuf(imgData);

        atlas->addImgForTile(id, imgData, imgInfo);

        tileUpdateNeeded = true;

        delete decoder;
        delete imgData;
}

TexCoords Dispatcher::getTileTexCoords(tileId id)
{
        return atlas->getTileTexCoords(id);
}

TexCoords Dispatcher::getParentTileTexCoords(tileId id)
{
        return atlas->getParentTileTexCoords(id);
}
void Dispatcher::removeTileFromAtlas(tileId id)
{
        atlas->removeTile(id);
}

void Dispatcher::markAllTilesUnused()
{
        atlas->markAllUnused();
}

void Dispatcher::removeUnusedTiles()
{
        atlas->removeUnused();
}
