//
// Created by Manuel Cabrerizo on 26/02/2023.
//

#include "tm_renderer.h"
#include "utils/tm_memory_pool.h"
#include "utils/tm_file.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android/imagedecoder.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

// TODO: memset zero all the return structs

#define TM_LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "App2", __VA_ARGS__))
#define TM_RENDERER_MEMORY_BLOCK_SIZE 100


struct TMBuffer {
    unsigned int id;
    unsigned int vbo;
    unsigned int ebo;
    TMVertex *vertices;
    unsigned int verticesCount;
    unsigned short *indices;
    unsigned int indicesCount;
};

struct TMShader {
    unsigned int id;
};

struct TMTexture {
    unsigned int id;
    int width;
    int height;
};

struct TMFramebuffer {
    unsigned int id;
};

struct TMRenderer {
    android_app *pApp;
    AAssetManager *assetManager;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLint width;
    EGLint height;

    TMMemoryPool *buffersMemory;
    TMMemoryPool *texturesMemory;
    TMMemoryPool *shadersMemory;
    TMMemoryPool *framebufferMemory;

};

#include <vector>

static void InitializeOpenGLContext(TMRenderer *renderer, android_app *pApp) {
    TM_LOG_INFO("Initilizing OpenGL ES 3 ...\n");

    // init opengl es 3
    renderer->pApp = pApp;
    renderer->display = EGL_NO_DISPLAY;
    renderer->surface = EGL_NO_SURFACE;
    renderer->context = EGL_NO_CONTEXT;
    renderer->width = 0;
    renderer->height = 0;

    EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // get the display
    renderer->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(renderer->display, NULL, NULL);

    // figure out how many config there are
    EGLint numConfigs;
    eglChooseConfig(renderer->display, attribs, NULL, 0, &numConfigs);

    // TODO: fix this
    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(renderer->display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    EGLDisplay display = renderer->display;
    void *selectedConfig = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    assert(selectedConfig != NULL);

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(renderer->display, selectedConfig, EGL_NATIVE_VISUAL_ID, &format);
    renderer->surface = eglCreateWindowSurface(renderer->display, selectedConfig, renderer->pApp->window, NULL);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    renderer->context = eglCreateContext(renderer->display, selectedConfig, NULL, contextAttribs);

    // make the context current
    EGLBoolean madeCurrent = eglMakeCurrent(renderer->display, renderer->surface, renderer->surface, renderer->context);

    if(madeCurrent) {
        TM_LOG_INFO("context made current\n");
    } else {
        TM_LOG_INFO("ERROR: context coudent make it\n");
    }

    assert(madeCurrent);

    // make width and height invalid so it gets updated the first frame
    renderer->width = -1;
    renderer->height = -1;

}

TMRenderer *TMRendererCreate(android_app *pApp, AAssetManager *assetManager) {
    TMRenderer *renderer = (TMRenderer *)malloc(sizeof(TMRenderer));

    InitializeOpenGLContext(renderer, pApp);

    renderer->buffersMemory = TMMemoryPoolCreate(sizeof(TMBuffer), TM_RENDERER_MEMORY_BLOCK_SIZE);
    renderer->texturesMemory = TMMemoryPoolCreate(sizeof(TMTexture), TM_RENDERER_MEMORY_BLOCK_SIZE);
    renderer->shadersMemory = TMMemoryPoolCreate(sizeof(TMShader), TM_RENDERER_MEMORY_BLOCK_SIZE);
    renderer->framebufferMemory = TMMemoryPoolCreate(sizeof(TMFramebuffer), TM_RENDERER_MEMORY_BLOCK_SIZE);
    renderer->assetManager = assetManager;


    glClearColor(0.5f, 0.1f, 0.1f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    eglSwapInterval(renderer->display, 1);

    return renderer;
}

void TMRendererDestroy(TMRenderer *renderer) {
    TMMemoryPoolDestroy(renderer->buffersMemory);
    TMMemoryPoolDestroy(renderer->texturesMemory);
    TMMemoryPoolDestroy(renderer->shadersMemory);
    TMMemoryPoolDestroy(renderer->framebufferMemory);
    free(renderer);
}

void TMRendererDepthTestEnable() {
    glEnable(GL_DEPTH_TEST);
}

void TMRendererDepthTestDisable() {
    glDisable(GL_DEPTH_TEST);
}

int TMRendererGetWidth(TMRenderer *renderer) {
    return renderer->width;
}
int TMRendererGetHeight(TMRenderer *renderer) {
    return renderer->height;
}

bool TMRendererUpdateRenderArea(TMRenderer *renderer) {
    EGLint width;
    eglQuerySurface(renderer->display, renderer->surface, EGL_WIDTH, &width);
    EGLint height;
    eglQuerySurface(renderer->display, renderer->surface, EGL_HEIGHT, &height);
    if(width != renderer->width || height != renderer->height) {
        renderer->width = width;
        renderer->height = height;
        glViewport(0, 0, width, height);
        return true;
    }
    return false;
}

void TMRendererFaceCulling(bool value,  unsigned int flags) {
    if(value) {
        glEnable(GL_CULL_FACE);
        if (flags == TM_CULL_BACK) {
            glCullFace(GL_BACK);
            return;
        }
        if (flags == TM_CULL_FRONT) {
            glCullFace(GL_FRONT);
            return;
        }
        if (flags == (TM_CULL_BACK | TM_CULL_FRONT)) {
            glCullFace(GL_FRONT_AND_BACK);
            return;
        }
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void TMRendererClear(float r, float g, float b, float a, unsigned  int flags) {
        glClearColor(r, g, b, a);
        unsigned int mask = 0;
        if(flags & TM_COLOR_BUFFER_BIT) mask |= GL_COLOR_BUFFER_BIT;
        if(flags & TM_DEPTH_BUFFER_BIT) mask |= GL_DEPTH_BUFFER_BIT;
        if(flags & TM_STENCIL_BUFFER_BIT) mask |= GL_STENCIL_BUFFER_BIT;
        glClear(mask);
}

void TMRendererPresent(TMRenderer *renderer) {
    EGLBoolean swapResult = eglSwapBuffers(renderer->display, renderer->surface);
    assert(swapResult == EGL_TRUE);
}

TMBuffer *TMRendererBufferCreate(TMRenderer *renderer,
                                 TMVertex *vertices, unsigned int verticesCount) {
    TMBuffer *buffer = (TMBuffer *)TMMemoryPoolAlloc(renderer->buffersMemory);

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TMVertex) * verticesCount, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TMVertex), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TMVertex), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    buffer->id = VAO;
    buffer->vbo = VBO;
    buffer->vertices = vertices;
    buffer->verticesCount = verticesCount;

    return buffer;

}

TMBuffer *TMRendererBufferCreate(TMRenderer *renderer,
                                 TMVertex *vertices, unsigned int verticesCount,
                                 unsigned short *indices, unsigned int indicesCount) {
    TMBuffer *buffer = (TMBuffer *)TMMemoryPoolAlloc(renderer->buffersMemory);

    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TMVertex) * verticesCount, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indicesCount, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TMVertex), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TMVertex), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    buffer->id = VAO;
    buffer->vbo = VBO;
    buffer->ebo = EBO;
    buffer->vertices = vertices;
    buffer->verticesCount = verticesCount;
    buffer->indices = indices;
    buffer->indicesCount = indicesCount;

    return buffer;

}

void TMRendererBufferDestroy(TMRenderer *renderer, TMBuffer *buffer) {
    glDeleteBuffers(1, &buffer->vbo);
    glDeleteBuffers(1, &buffer->ebo);
    glDeleteVertexArrays(1, &buffer->id);
    TMMemoryPoolFree(renderer->buffersMemory, (void *)buffer);
}

void TMRendererDrawBufferElements(TMBuffer *buffer) {
    glBindVertexArray(buffer->id);
    glDrawElements(GL_TRIANGLES, buffer->indicesCount, GL_UNSIGNED_SHORT, 0);
}

void TMRendererDrawBufferArray(TMBuffer *buffer) {
    glBindVertexArray(buffer->id);
    glDrawArrays(GL_TRIANGLES, 0, buffer->verticesCount);
}

TMShader *TMRendererShaderCreate(TMRenderer *renderer, const char *vertPath, const char *fragPath) {
    TMShader *shader = (TMShader *)TMMemoryPoolAlloc(renderer->shadersMemory);

    // TODO: change this to work with files ...
    TMFile vertFile = TMFileOpen(renderer->assetManager, vertPath);
    TMFile fragFile = TMFileOpen(renderer->assetManager, fragPath);
    const char *vertSource = (const char *)vertFile.data;
    const char *fragSource = (const char *)fragFile.data;

    // TODO: ...
    int success;
    char infoLog[512];

    // compile and create the vertex shader
    unsigned int vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSource, NULL);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED:\n %s\n", infoLog);
    }

    // compile and create the fragment shader
    unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED:\n %s\n", infoLog);
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n %s\n", infoLog);
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    shader->id = program;

    TMFileClose(&vertFile);
    TMFileClose(&fragFile);

    return shader;
}

void TMRendererShaderDestroy(TMRenderer *renderer, TMShader *shader) {
    glDeleteShader(shader->id);
    TMMemoryPoolFree(renderer->shadersMemory, (void *)shader);
}

void TMRendererBindShader(TMShader *shader) {
    glUseProgram(shader->id);
}

void TMRendererUnbindShader(TMShader *shader) {
    glUseProgram(0);
}

void TMRendererShaderUpdate(TMShader *shader, const char *varName, float value) {
    int varLoc = glGetUniformLocation(shader->id, varName);
    TMRendererBindShader(shader);
    glUniform1f(varLoc, value);
}

void TMRendererShaderUpdate(TMShader *shader, const char *varName, int value) {
    int varLoc = glGetUniformLocation(shader->id, varName);
    TMRendererBindShader(shader);
    glUniform1i(varLoc, value);
}

void TMRendererShaderUpdate(TMShader *shader, const char *varName, TMVec3 value) {
    int varLoc = glGetUniformLocation(shader->id, varName);
    TMRendererBindShader(shader);
    glUniform3fv(varLoc, 1, value.v);
}

void TMRendererShaderUpdate(TMShader *shader, const char *varName, TMVec4 value) {
    int varLoc = glGetUniformLocation(shader->id, varName);
    TMRendererBindShader(shader);
    glUniform4fv(varLoc, 1, value.v);
}

void TMRendererShaderUpdate(TMShader *shader, const char *varName, TMMat4 value) {
    int varLoc = glGetUniformLocation(shader->id, varName);
    TMRendererBindShader(shader);
    glUniformMatrix4fv(varLoc, 1, false, value.v);
}

void TMRendererShaderUpdate(TMShader *shader, const char *varName, int size, int *array) {
    int varLoc = glGetUniformLocation(shader->id, varName);
    TMRendererBindShader(shader);
    glUniform1iv(varLoc, size, array);
}

void TMRendererShaderUpdate(TMShader *shader, const char *varName, int size, TMMat4 *array) {
    int varLoc = glGetUniformLocation(shader->id, varName);
    TMRendererBindShader(shader);
    glUniformMatrix4fv(varLoc, size, false, (float *)array);
}

TMTexture *TMRendererTextureCreate(TMRenderer *renderer, const char *filepath) {
    TMTexture *texture = (TMTexture *)TMMemoryPoolAlloc(renderer->texturesMemory);

    AAsset *file = AAssetManager_open(renderer->assetManager, filepath, AASSET_MODE_BUFFER);

    // manuel: make a decoder to turn in into a texture
    AImageDecoder *androidDecoder = NULL;
    int result = AImageDecoder_createFromAAsset(file, &androidDecoder);
    assert(result == ANDROID_IMAGE_DECODER_SUCCESS);

    // manuel: make sure we get 8 bits per channel RGBA
    AImageDecoder_setAndroidBitmapFormat(androidDecoder, ANDROID_BITMAP_FORMAT_RGBA_8888);

    const AImageDecoderHeaderInfo *header = NULL;
    header = AImageDecoder_getHeaderInfo(androidDecoder);

    int width = AImageDecoderHeaderInfo_getWidth(header);
    int height = AImageDecoderHeaderInfo_getHeight(header);
    int stride = AImageDecoder_getMinimumStride(androidDecoder);

    // TODO(manuel): fix this ...
#if 0
    // manuel: alloc memory for the texture
    unsigned int *imageData = (unsigned int *)malloc(width * height * sizeof(unsigned int));
    int decodeResult = AImageDecoder_decodeImage(androidDecoder, imageData, stride, width*height);
    assert(decodeResult == ANDROID_IMAGE_DECODER_SUCCESS);
#else if
    // Get the bitmap data of the image
    auto upAndroidImageData = std::make_unique<std::vector<uint8_t>>(height * stride);
    auto decodeResult = AImageDecoder_decodeImage(
            androidDecoder,
            upAndroidImageData->data(),
            stride,
            upAndroidImageData->size());
    assert(decodeResult == ANDROID_IMAGE_DECODER_SUCCESS);
#endif
    // manuel: create opengl texture
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // manuel: Clamp to the edge, you'll get odd results alpha blending if you don't
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // manuel: Load the texture into VRAM
    glTexImage2D(
            GL_TEXTURE_2D, // target
            0, // mip level
            GL_RGBA, // internal format, often advisable to use BGR
            width, // width of the texture
            height, // height of the texture
            0, // border (always 0)
            GL_RGBA, // format
            GL_UNSIGNED_BYTE, // type
            upAndroidImageData->data() // Data to upload
    );

    // manuel: generate mip levels. Not really needed for 2D, but good to do
    glGenerateMipmap(GL_TEXTURE_2D);

    AImageDecoder_delete(androidDecoder);
    AAsset_close(file);

    texture->id = textureId;
    texture->width = width;
    texture->height = height;

    return texture;
}

void TMRendererTextureBind(TMTexture *texture, TMShader *shader, const char *varName, int textureIndex) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    TMRendererShaderUpdate(shader, varName, textureIndex);
}

void TMRendererTextureUnbind(TMTexture *texture, int textureIndex) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}

void TMRendererTextureDestroy(TMRenderer *renderer, TMTexture *texture) {
    glDeleteTextures(1, &texture->id);
    TMMemoryPoolFree(renderer->texturesMemory, (void *)texture);
}

TMFramebuffer *TMRendererFramebufferCreate(TMRenderer *renderer) {
    TMFramebuffer *framebuffer = (TMFramebuffer *)TMMemoryPoolAlloc(renderer->framebufferMemory);
    return framebuffer;
}

void TMRendererFramebufferDestroy(TMRenderer *renderer, TMFramebuffer *framebuffer) {
    TMMemoryPoolFree(renderer->framebufferMemory, (void *)framebuffer);
}
