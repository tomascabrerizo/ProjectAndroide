//
// Created by Manuel Cabrerizo on 27/02/2023.
//

#include "tm_input.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <memory.h>

#define TM_LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "App2", __VA_ARGS__))

void TMInputInitialize(TMInput *input) {
    memset(input, 0, sizeof(TMInput));
}

void TMInputHandle(TMInput *input, android_app *pApp) {
    // handle all queued inputs
    for (auto i = 0; i < pApp->motionEventsCount; i++) {
        // cache the current event
        auto &motionEvent = pApp->motionEvents[i];

        // cache the current action
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        // get the x and y position of this event
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);
        input->currMotions[i].x = x;
        input->currMotions[i].y = y;

        // Only consider touchscreen events, like touches
        auto actionMasked = action & AINPUT_SOURCE_TOUCHSCREEN;

        // determine the kind of event it is
        switch (actionMasked) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                break;

            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                break;

            default:
                break;
        }

    }
    // clear inputs, be careful as this will clear it for anyone listening to these events
    android_app_clear_motion_events(pApp);
}

TMVec2 TMInputGetCurrentMotionByIndex(TMInput *input, unsigned int index) {
    return input->currMotions[index];
}

TMVec2 TMInputGetLastMotionByIndex(TMInput *input, unsigned int index) {
    return input->lastMotions[index];
}
