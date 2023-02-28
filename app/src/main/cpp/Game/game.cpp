//
// Created by Manuel Cabrerizo on 27/02/2023.
//

#include "game.h"
#include "models.h"

#include <time.h>
#include <stdlib.h>

static void UpdateProjectionsMatrices(GameState *state) {
    // manuel: create the projection and view matrix
    int width = TMRendererGetWidth(state->renderer);
    int height = TMRendererGetHeight(state->renderer);
    state->orthographic = TMMat4Ortho(-width/2, width/2, -height/2, height/2, 0, 100);
    state->perspective = TMMat4Perspective(60.0f, (float)width/(float)height, 0.01f, 100.0f);
}

static void UpdateViewMatrix(GameState *state) {
    TMVec3 position{0, 0, 10};
    TMVec3 target{0, 0, 0};
    TMVec3 up{0, 1, 0};
    state->view = TMMat4LookAt(position, target, up);
    TMRendererShaderUpdate(state->shader, "uView", state->view);
}

static void InitializeEntities(GameState *state) {
    // manuel: init player1
    state->player1Size.x = 400;
    state->player1Size.y = 100;
    state->player1Position.x = -200;
    state->player1Position.y = 800;
    // manuel: init player2
    state->player2Size.x = 400;
    state->player2Size.y = 100;
    state->player2Position.x = 400;
    state->player2Position.y = -800;
    // manuel: init ball
    state->ballPosition.x = 0;
    state->ballPosition.y = 0;
    state->ballVelocity.x = rand() / 2.0f;
    state->ballVelocity.y = rand();
    TMVec2Normalize(&state->ballVelocity);
    state->ballVelocity = state->ballVelocity * 40;
    state->ballSize.x = 200;
    state->ballSize.y = 200;
}

static void CollisionDetectionAndResolution(GameState *state, int width, int height) {
    TMVec2 halfBallSize = state->ballSize*0.5f;
    TMVec2 halfPlayerSize = state->player1Size*0.5f;
    TMVec2 player1 = state->player1Position;
    TMVec2 player2 = state->player2Position;
    TMVec2 ball = state->ballPosition;
    if(ball.x - halfBallSize.x <= player1.x + halfPlayerSize.x &&
       ball.x + halfBallSize.x >= player1.x - halfPlayerSize.x &&
       ball.y - halfBallSize.y <= player1.y + halfPlayerSize.y &&
       ball.y + halfBallSize.y >= player1.y - halfPlayerSize.y) {
        if(state->ballVelocity.y > 0.0f) {
            state->ballVelocity.y = -state->ballVelocity.y;
        }
    }
    if(ball.x - halfBallSize.x <= player2.x + halfPlayerSize.x &&
       ball.x + halfBallSize.x >= player2.x - halfPlayerSize.x &&
       ball.y - halfBallSize.y <= player2.y + halfPlayerSize.y &&
       ball.y + halfBallSize.y >= player2.y - halfPlayerSize.y) {
        if(state->ballVelocity.y < 0.0f) {
            state->ballVelocity.y = -state->ballVelocity.y;
        }    }
    if(state->ballPosition.x <= -width/2) {
        state->ballPosition.x = (-width/2) + 1;
        state->ballVelocity.x = -state->ballVelocity.x;
    }
    if(state->ballPosition.x >= width/2) {
        state->ballPosition.x = (width/2) - 1;
        state->ballVelocity.x = -state->ballVelocity.x;
    }
    if(state->ballPosition.y <= -height/2) {
        state->ballPosition.y = (-height/2) + 1;
        state->ballVelocity.y = -state->ballVelocity.y;
    }
    if(state->ballPosition.y >= height/2) {
        state->ballPosition.y = (height/2) - 1;
        state->ballVelocity.y = -state->ballVelocity.y;
    }
}

void GameInitialize(GameState *state, android_app *pApp, AAssetManager *assetManager) {
    state->renderer = TMRendererCreate(pApp, assetManager);

    state->shader = TMRendererShaderCreate(state->renderer,
                                           "shaders/vert.glsl",
                                           "shaders/frag.glsl");

    state->buffer = TMRendererBufferCreate(state->renderer,
                                           vertices, ARRAY_LENGTH(vertices),
                                           indices, ARRAY_LENGTH(indices));
    state->cubeBuffer = TMRendererBufferCreate(state->renderer, cubeVertices, ARRAY_LENGTH(cubeVertices));


    state->donutTexture = TMRendererTextureCreate(state->renderer, "images/donut.png");
    state->backgroundTexture = TMRendererTextureCreate(state->renderer, "images/back.png");
    state->paddle1Texture = TMRendererTextureCreate(state->renderer, "images/paddle_1.png");
    state->paddle2Texture = TMRendererTextureCreate(state->renderer, "images/paddle_2.png");
    state->moonTexture = TMRendererTextureCreate(state->renderer, "images/moon.png");

    UpdateProjectionsMatrices(state);
    UpdateViewMatrix(state);
    TMRendererFaceCulling(false, 0);

    // manuel: Initializes random number generator
    time_t t;
    srand((unsigned) time(&t));

    InitializeEntities(state);

}

void GameUpdate(GameState *state, TMInput *input, float dt) {
    int width = TMRendererGetWidth(state->renderer);
    int height = TMRendererGetHeight(state->renderer);

    TMVec2 currMotion0 = TMInputGetCurrentMotionByIndex(input, 0);
    TMVec2 currMotion1 = TMInputGetCurrentMotionByIndex(input, 0);

    state->player1Position.x = currMotion0.x - TMVec2{ width*0.5f, height*0.5f }.x;
    state->player2Position.x = currMotion1.x - TMVec2{ width*0.5f, height*0.5f }.x;

    CollisionDetectionAndResolution(state, width, height);

    state->ballPosition = state->ballPosition + state->ballVelocity;
}

void GameRender(GameState *state) {

    if(TMRendererUpdateRenderArea(state->renderer)) {
        UpdateProjectionsMatrices(state);
    }

    TMRendererClear(0.1f, 0.5f, 0.1f, 1.0f, TM_COLOR_BUFFER_BIT|TM_DEPTH_BUFFER_BIT);

    // manuel: set the shader
    TMRendererBindShader(state->shader);
    TMRendererShaderUpdate(state->shader, "uProj", state->orthographic);

    // manuel: render the background
    int width = TMRendererGetWidth(state->renderer);
    int height = TMRendererGetHeight(state->renderer);
    static float angle = 0.0f;

    TMMat4 scale = TMMat4Identity();
    TMMat4 trans = TMMat4Identity();
    TMMat4 rotat = TMMat4Identity();
    TMMat4 world = TMMat4Identity();

    scale = TMMat4Scale(width, height, 1);
    world = scale;
    TMRendererShaderUpdate(state->shader, "uWorld", world);
    // manuel: set the background texture
    TMRendererTextureBind(state->backgroundTexture, state->shader, "uTexture", 0);
    // manuel: draw the background
    TMRendererDrawBufferElements(state->buffer);
    // manuel: draw player one
    scale = TMMat4Scale(state->player1Size.x, state->player1Size.y, 1);
    trans = TMMat4Translate(state->player1Position.x, state->player1Position.y, 0);
    world = trans * scale;
    TMRendererShaderUpdate(state->shader, "uWorld", world);
    TMRendererTextureBind(state->paddle1Texture, state->shader, "uTexture", 0);
    TMRendererDrawBufferElements(state->buffer);
    // manuel: draw player two
    scale = TMMat4Scale(state->player2Size.x, state->player2Size.y, 1);
    trans = TMMat4Translate(state->player2Position.x, state->player2Position.y, 0);
    world = trans * scale;
    TMRendererShaderUpdate(state->shader, "uWorld", world);
    TMRendererTextureBind(state->paddle2Texture, state->shader, "uTexture", 0);
    TMRendererDrawBufferElements(state->buffer);
    // manuel: draw donut
    scale = TMMat4Scale(state->ballSize.x, state->ballSize.y, 1);
    trans = TMMat4Translate(state->ballPosition.x, state->ballPosition.y, 0);
    rotat = TMMat4RotateZ(angle);
    world = trans *  rotat * scale;
    TMRendererShaderUpdate(state->shader, "uWorld", world);
    TMRendererTextureBind(state->donutTexture, state->shader, "uTexture", 0);
    TMRendererDrawBufferElements(state->buffer);


    // manuel: draw the 3d cube
    TMRendererShaderUpdate(state->shader, "uProj", state->perspective);
    TMRendererDepthTestEnable();

    trans = TMMat4Translate(2, 4, 0);
    rotat = TMMat4RotateY(angle) * TMMat4RotateX(angle);
    world = trans * rotat;
    TMRendererShaderUpdate(state->shader, "uWorld", world);
    TMRendererTextureBind(state->moonTexture, state->shader, "uTexture", 0);
    TMRendererDrawBufferArray(state->cubeBuffer);

    TMRendererDepthTestDisable();

    angle += 0.02f;

    TMRendererPresent(state->renderer);
}

void GameShutdown(GameState *state) {
    TMRendererTextureDestroy(state->renderer, state->moonTexture);
    TMRendererTextureDestroy(state->renderer, state->donutTexture);
    TMRendererTextureDestroy(state->renderer, state->backgroundTexture);
    TMRendererTextureDestroy(state->renderer, state->paddle1Texture);
    TMRendererTextureDestroy(state->renderer, state->paddle2Texture);
    TMRendererBufferDestroy(state->renderer, state->cubeBuffer);
    TMRendererBufferDestroy(state->renderer, state->buffer);
    TMRendererShaderDestroy(state->renderer, state->shader);
    TMRendererDestroy(state->renderer);
}