#include <jni.h>

#include "TMEngine/tm_input.h"
#include "Game/game.h"


// NOTE: No Delete
#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C" {

#include <game-activity/native_app_glue/android_native_app_glue.c>

/*!
 * Handles commands sent to this Android application
 * @param pApp the app the commands are coming from
 * @param cmd the command to handle
 */

void handle_cmd(android_app *pApp, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            pApp->userData = (void *) malloc(sizeof(GameState));
            GameState *gameState = (GameState *) pApp->userData;
            GameInitialize(gameState, pApp, pApp->activity->assetManager);
        } break;
        case APP_CMD_TERM_WINDOW: {
            if (pApp->userData) {
                GameState *gameState = (GameState *) pApp->userData;
                GameShutdown(gameState);
                free(pApp->userData);
                pApp->userData = NULL;
            }
        } break;
        default:
            break;
    }
}

void android_poll_events(android_app *pApp) {
    int events;
    android_poll_source *pSource;
    if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0) {
        if (pSource) {
            pSource->process(pApp, pSource);
        }
    }
}

/*!
 * This the main entry point for a native activity
 */
void android_main(struct android_app *pApp) {
    // register an event handler for Android events
    pApp->onAppCmd = handle_cmd;

    TMInput input;
    TMInputInitialize(&input);

    while(!pApp->destroyRequested) {
        android_poll_events(pApp);
        if(pApp->userData) {
            GameState *gameState = (GameState *)pApp->userData;

            TMInputHandle(&input, pApp);

            GameUpdate(gameState, &input, 0);

            GameRender(gameState);

            for(int i = 0; i < 16; ++i) {
                input.lastMotions[i] = input.currMotions[i];
            }
        }
    }

}


}
