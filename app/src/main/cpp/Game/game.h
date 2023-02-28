//
// Created by Manuel Cabrerizo on 27/02/2023.
//

#ifndef MY_APPLICATION_GAME_H
#define MY_APPLICATION_GAME_H

#include <android/log.h>
#include "../TMEngine//tm_renderer.h"
#include "../TMEngine/tm_input.h"


#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))
#define TM_LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "App2", __VA_ARGS__))

struct android_app;
struct AAssetManager;

struct GameState {
    TMRenderer *renderer;
    TMShader *shader;

    TMBuffer *buffer;
    TMBuffer *cubeBuffer;

    TMTexture *donutTexture;
    TMTexture *backgroundTexture;
    TMTexture *paddle1Texture;
    TMTexture *paddle2Texture;
    TMTexture *moonTexture;

    TMMat4 perspective;
    TMMat4 orthographic;
    TMMat4 view;

    TMVec2 player1Position;
    TMVec2 player1Size;
    TMVec2 player2Position;
    TMVec2 player2Size;
    TMVec2 ballPosition;
    TMVec2 ballVelocity;
    TMVec2 ballSize;

};

void GameInitialize(GameState *state, android_app *pApp, AAssetManager *assetManager);
void GameUpdate(GameState *state, TMInput *input, float dt);
void GameRender(GameState *state);
void GameShutdown(GameState *state);

#endif //MY_APPLICATION_GAME_H
