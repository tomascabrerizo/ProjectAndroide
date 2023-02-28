//
// Created by Manuel Cabrerizo on 27/02/2023.
//

#ifndef MY_APPLICATION_TM_INPUT_H
#define MY_APPLICATION_TM_INPUT_H

#include "utils/tm_math.h"

struct android_app;

struct TMInput {
    TMVec2 currMotions[16];
    TMVec2 lastMotions[16];
};

void TMInputInitialize(TMInput *input);
void TMInputHandle(TMInput *input, android_app *pApp);
TMVec2 TMInputGetCurrentMotionByIndex(TMInput *input, unsigned int index);
TMVec2 TMInputGetLastMotionByIndex(TMInput *input, unsigned int index);

#endif //MY_APPLICATION_TM_INPUT_H
