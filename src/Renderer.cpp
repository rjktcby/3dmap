#include "Renderer.h"
#include <iostream>
#include <string>
#include <sstream>

#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Dispatcher.h"
#include "tileId.hpp"

// #define _RENDERER_DEBUG 1

Renderer::Renderer(Dispatcher *d, int width, int height)
        : dispatcher(d), translation(0.0f), rotation(0.0f),
        zoomLevel(2.0f), zoomFactor(1.0f)
{
        reshape(width, height);

        glEnable(GL_NORMALIZE);
}

Renderer::~Renderer()
{
        removeTiles();
}

void Renderer::removeTiles()
{
        std::list<Tile *>::iterator i;
        for (i = tiles.begin(); i != tiles.end(); ++i) {
                delete *i;
        }
        tiles.clear();
}

#define MAX_COORD 10.0f

#define MIN_X -MAX_COORD
#define MAX_X MAX_COORD
#define MIN_Z -MAX_COORD
#define MAX_Z MAX_COORD
#define OPTIMAL_AREA 65536.0f

#define TILE_MAX_Z 19

#define MAXIMUM_ZOOM_LEVEL 19
#define MINIMUM_ZOOM_LEVEL 0

void Renderer::tryAddTile(tileId id, glm::vec3 pos, float tile_size)
{
        Tile *tile = new Tile(id, pos, tile_size);

        float area = tile->screenArea(this);

        if (area == 0.0f) {
                dispatcher->removeTileFromAtlas(id);
                return;
        }

#ifdef _RENDERER_DEBUG
        std::cout << "Tile=" << id << " area=" << area << std::endl;
#endif

        if ( (abs(OPTIMAL_AREA - area) >= abs(OPTIMAL_AREA - area*0.25f) || (area > OPTIMAL_AREA) ) &&
             (id.z() < TILE_MAX_Z) )
        {
                dispatcher->removeTileFromAtlas(id);
                delete tile;

                float subtile_size = tile_size * 0.5f;
                tryAddTile(tileId(id.z()+1, id.x()*2, id.y()*2), pos, subtile_size);
                tryAddTile(tileId(id.z()+1, id.x()*2+1, id.y()*2), glm::vec3(pos[0]+subtile_size, pos[1], pos[2]), subtile_size);
                tryAddTile(tileId(id.z()+1, id.x()*2, id.y()*2+1), glm::vec3(pos[0], pos[1], pos[2]+subtile_size), subtile_size);
                tryAddTile(tileId(id.z()+1, id.x()*2+1, id.y()*2+1), glm::vec3(pos[0]+subtile_size, pos[1], pos[2]+subtile_size), subtile_size);
        } else {
                TexCoords texCoords = dispatcher->getTileTexCoords(id);

                if (texCoords.texId == 0) {
                        dispatcher->addTileFetchRequest(id);
                        texCoords = dispatcher->getParentTileTexCoords(id);
                }
                if (texCoords.texId != 0) {

#ifdef _RENDERER_DEBUG
                        std::cout << "Adding tile=" << id << std::endl;
#endif
                        tile->setTextureCoords(texCoords);

                        tile->setColor(glm::vec4(1.0f));
                        tiles.push_back(tile);
                }
        }
}

void Renderer::updateTiles()
{
        removeTiles();

        float x = 0.0f, y = 0.0f, z = 0.0f, tile_size = 10.0f;

        x = MIN_X; z = MIN_Z;
        while (x < MAX_X) {
                z = MIN_Z;
                while (z < MAX_Z) {
                        unsigned int tileX = (unsigned int)((x-MIN_X)/tile_size);
                        unsigned int tileY = (unsigned int)((z-MIN_Z)/tile_size);

                        tileId id = tileId(1, tileX, tileY);
                        tryAddTile(id, glm::vec3(x, y, z), tile_size);

                        z += tile_size;
                }
                x += tile_size;
        }

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] Total tiles: " << tiles.size() << std::endl;
#endif
}

void Renderer::updateModelViewMatrix()
{
        float eye[3]  = {0.0f, 6.0f, 6.0f};
        float look[3]  = {0.0f, 0.0f, 0.0f};
        float up[3]  = {0.0f, 1.0f, 0.0f};
        mViewMatrix = glm::lookAt(glm::make_vec3(eye), glm::make_vec3(look), glm::make_vec3(up));

        mModelMatrix = glm::mat4(1.0f);
        mModelMatrix = glm::scale(mModelMatrix, glm::vec3(zoomFactor));
        mModelMatrix = glm::rotate(mModelMatrix, rotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
        mModelMatrix = glm::rotate(mModelMatrix, rotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
        mModelMatrix = glm::rotate(mModelMatrix, rotation[2], glm::vec3(0.0f, 0.0f, 1.0f));
        mModelMatrix = glm::translate(mModelMatrix, translation);

        mModelViewMatrix = mViewMatrix * mModelMatrix;

        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf(glm::value_ptr(mModelViewMatrix));

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] mModelViewMatrix:" << std::endl;
        for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                        std::cout << mModelViewMatrix[i][j] << " ";
                }
                std::cout << std::endl;
        }
#endif
}

void Renderer::updateProjectionMatrix()
{
        const float iratio = (float) mViewport[3] / (float) mViewport[2];
        const float near = 1.0f;
        const float far = 100.0f;
        float xmax = near * 0.5f;

        mProjMatrix = glm::frustum(-xmax, xmax, -xmax*iratio, xmax*iratio, near, far); // near = 1.0f; far = 100.0f;

        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf(glm::value_ptr(mProjMatrix));

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] mProjMatrix:" << std::endl;
        for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                        std::cout << mProjMatrix[i][j] << " ";
                }
                std::cout << std::endl;
        }
#endif
}

void Renderer::updateMVPMatrix()
{
        mMVPMatrix = mProjMatrix * mModelViewMatrix;
        mInvMVPMatrix = glm::inverse(mMVPMatrix);
}

void Renderer::reshape(int width, int height)
{
        glViewport( 0, 0, (GLint) width, (GLint) height );
        mViewport = glm::vec4(0, 0, width, height);

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] reshape=" << mViewport << std::endl;
#endif

        updateProjectionMatrix();
        updateModelViewMatrix();
        updateMVPMatrix();
}

void Renderer::screenToWorld(glm::vec2 screen, glm::vec3 &world)
{
        int width = mViewport[2];
        int height = mViewport[3];

        float xx = (2.0f * screen[0]) / width - 1.0f;
        float yy = (2.0f * (height-screen[1])) / height - 1.0f;

        float zzN = 0.0f;
        float zzF = 100.0f;

#ifdef USE_GLM_UNPROJECT
        glm::vec3 nn = glm::unProject(glm::vec3(screen[0], height - screen[1], 0.0f), mModelViewMatrix, mProjMatrix, glm::vec4(0, 0, width, height));
        glm::vec3 ff = glm::unProject(glm::vec3(screen[0], height - screen[1], 1.0f), mModelViewMatrix, mProjMatrix, glm::vec4(0, 0, width, height));
#else
        glm::vec4 mmN = mInvMVPMatrix * glm::vec4(xx, yy, zzN, 1.0f);
        glm::vec4 mmF = mInvMVPMatrix * glm::vec4(xx, yy, zzF, 1.0f);

        glm::vec3 nn = glm::vec3(mmN[0] / mmN[3], mmN[1] / mmN[3], mmN[2] / mmN[3]);
        glm::vec3 ff = glm::vec3(mmF[0] / mmF[3], mmF[1] / mmF[3], mmF[2] / mmF[3]);
#endif

        float t = nn[1] / (nn[1] - ff[1]);

        world[0] = nn[0] + (ff[0] - nn[0]) * t;
        world[1] = 0.0f;
        world[2] = nn[2] + (ff[2] - nn[2]) * t;
}

void Renderer::worldToScreen(glm::vec3 world, glm::vec2 &screen)
{
        glm::vec4 tmptmp = mModelViewMatrix * glm::vec4(world, 1.0);
        glm::vec4 tmp = mProjMatrix * tmptmp;

        if (tmp[3] < 0.0f) {
                tmp[0] = tmp[0] / abs(tmp[3]) * 2.0f;
                tmp[1] = tmp[1] / abs(tmp[3]) * 2.0f;
        } else {
                tmp[0] = tmp[0] / tmp[3];
                tmp[1] = tmp[1] / tmp[3];
        }

        tmp[0] = tmp[0] * 0.5 + 0.5;
        tmp[1] = tmp[1] * 0.5 + 0.5;

        screen[0] = tmp[0] * mViewport[2];
        screen[1] = (1.0 - tmp[1]) * mViewport[3];

        // glm::vec3 ppn = glm::project(world, mModelViewMatrix, mProjMatrix, mViewport);
        // screen[0] = ppn[0]; screen[1] = mViewport[3] - ppn[1];
}


bool tileSortFunc(Tile *a, Tile *b) {
        return a->getTexId() < b->getTexId();
}

void Renderer::callBatch(struct TileVertex *vertices, int nTiles, GLuint texId)
{

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] Batched " << nTiles << " tiles with texId=" << texId << std::endl;
#endif
        if (nTiles == 0)
                return;

        glBindTexture(GL_TEXTURE_2D, texId);

        glVertexPointer(3, GL_FLOAT, sizeof(TileVertex), &(vertices[0].x));
        glColorPointer(3, GL_FLOAT, sizeof(TileVertex), &(vertices[0].cR));
        glTexCoordPointer(2, GL_FLOAT, sizeof(TileVertex), &(vertices[0].tx));

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glClientActiveTexture(GL_TEXTURE0);

        glDrawArrays(GL_QUADS, 0, nTiles*4);

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Renderer::draw()
{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPushMatrix();

                glEnable(GL_TEXTURE_2D);

                int nTiles = tiles.size();
                if (nTiles == 0) {
                        glPopMatrix();
                        return;
                }

                struct TileVertex *vertices = (struct TileVertex *)malloc(sizeof(TileVertex)*nTiles*4);

                tiles.sort(tileSortFunc);

                dispatcher->markAllTilesUnused();

                std::list<Tile *>::iterator i;
                int offset = 0, tilesInBatch = 0;
                Tile *firstTile = *(tiles.begin());
                GLuint batchTexId = firstTile ? firstTile->getTexId() : 0;

                for (i = tiles.begin(); i != tiles.end(); ++i) {
                        GLuint currentTexId = (*i)->getTexId();
                        if (currentTexId != batchTexId) {
                                callBatch(vertices, tilesInBatch, batchTexId);
                                offset = 0; tilesInBatch = 0;
                                batchTexId = currentTexId;
                        }

                        (*i)->appendToBatch(&(vertices[offset]));
                        offset += 4;
                        tilesInBatch++;
                }

                callBatch(vertices, tilesInBatch, batchTexId);
                dispatcher->removeUnusedTiles();

                free(vertices);

        glPopMatrix();
}

void Renderer::rotate(glm::vec3 diff)
{
        rotation += diff;

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] New rotation=" << rotation << std::endl;
#endif
        updateModelViewMatrix();
        updateMVPMatrix();
}

void Renderer::translate(glm::vec2 from, glm::vec2 to)
{
        glm::vec3 worldFrom, worldTo;

        screenToWorld(from, worldFrom);
        screenToWorld(to, worldTo);

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] translate " << worldFrom << " -- " << worldTo << std::endl;
#endif

        translation += worldTo - worldFrom;

        updateModelViewMatrix();
        updateMVPMatrix();
}

void Renderer::zoomUp()
{
        int newZoomLevel = zoomLevel++;

        if (zoomLevel > MAXIMUM_ZOOM_LEVEL)
                return;

        zoomLevel = newZoomLevel; zoomFactor *= 2.0f;

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] New zoom: level=" << newZoomLevel << " factor=" << zoomFactor << std::endl;
#endif

        updateModelViewMatrix();
        updateMVPMatrix();
}

void Renderer::zoomDown()
{
        int newZoomLevel = zoomLevel--;

        if (zoomLevel < MINIMUM_ZOOM_LEVEL)
                return;

        zoomLevel = newZoomLevel; zoomFactor *= 0.5f;

#ifdef _RENDERER_DEBUG
        std::cout << "[Renderer] New zoom: level=" << newZoomLevel << " factor=" << zoomFactor << std::endl;
#endif
        updateModelViewMatrix();
        updateMVPMatrix();
}

glm::vec4 Renderer::getViewport() const
{
        return mViewport;
}
