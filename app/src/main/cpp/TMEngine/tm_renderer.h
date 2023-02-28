//
// Created by Manuel Cabrerizo on 26/02/2023.
//

#ifndef MY_APPLICATION_TM_SHADER_H
#define MY_APPLICATION_TM_SHADER_H

#define TM_COLOR_BUFFER_BIT   (1 << 0)
#define TM_DEPTH_BUFFER_BIT   (1 << 1)
#define TM_STENCIL_BUFFER_BIT (1 << 2)

#define TM_CULL_BACK (1 << 0)
#define TM_CULL_FRONT (1 << 1)

#include "utils/tm_math.h"

struct android_app;
struct AAssetManager;

struct TMRenderer;
struct TMBuffer;
struct TMShader;
struct TMTexture;
struct TMFramebuffer;

struct TMVertex {
    TMVec3 position;
    TMVec2 uv;
};

TMRenderer *TMRendererCreate(android_app *pApp, AAssetManager *assetManager);
void TMRendererDestroy(TMRenderer *renderer);
void TMRendererDepthTestEnable();
void TMRendererDepthTestDisable();
void TMRendererFaceCulling(bool value,  unsigned int flags);
int TMRendererGetWidth(TMRenderer *renderer);
int TMRendererGetHeight(TMRenderer *renderer);
bool TMRendererUpdateRenderArea(TMRenderer *renderer);
void TMRendererClear(float r, float g, float b, float a, unsigned  int flags);
void TMRendererPresent(TMRenderer *renderer);


TMBuffer *TMRendererBufferCreate(TMRenderer *renderer,
                                 TMVertex *vertices, unsigned int verticesCount);
TMBuffer *TMRendererBufferCreate(TMRenderer *renderer,
                                 TMVertex *vertices, unsigned int verticesCount,
                                 unsigned short *indices, unsigned int indicesCount);
void TMRendererBufferDestroy(TMRenderer *renderer, TMBuffer *buffer);
void TMRendererDrawBufferElements(TMBuffer *buffer);
void TMRendererDrawBufferArray(TMBuffer *buffer);

TMShader *TMRendererShaderCreate(TMRenderer *renderer, const char *vertPath, const char *fragPath);
void TMRendererShaderDestroy(TMRenderer *renderer, TMShader *shader);
void TMRendererBindShader(TMShader *shader);
void TMRendererShaderUpdate(TMShader *shader, const char *varName, float value);
void TMRendererShaderUpdate(TMShader *shader, const char *varName, int value);
void TMRendererShaderUpdate(TMShader *shader, const char *varName, TMVec3 value);
void TMRendererShaderUpdate(TMShader *shader, const char *varName, TMVec4 value);
void TMRendererShaderUpdate(TMShader *shader, const char *varName, TMMat4 value);
void TMRendererShaderUpdate(TMShader *shader, const char *varName, int size, int *array);
void TMRendererShaderUpdate(TMShader *shader, const char *varName, int size, TMMat4 *array);

TMTexture *TMRendererTextureCreate(TMRenderer *renderer, const char *filepath);
void TMRendererTextureDestroy(TMRenderer *renderer, TMTexture *texture);
void TMRendererTextureBind(TMTexture *texture, TMShader *shader, const char *varName, int textureIndex);
void TMRendererTextureUnbind(TMTexture *texture, int textureIndex);


TMFramebuffer *TMRendererFramebufferCreate(TMRenderer *renderer);
void TMRendererFramebufferDestroy(TMRenderer *renderer, TMFramebuffer *framebuffer);

#endif //MY_APPLICATION_TM_SHADER_H
