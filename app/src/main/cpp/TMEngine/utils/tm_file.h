//
// Created by Manuel Cabrerizo on 26/02/2023.
//

#ifndef MY_APPLICATION_TM_FILE_H
#define MY_APPLICATION_TM_FILE_H

#include <stddef.h>
#include <android/asset_manager.h>

struct TMFile {
    void *data;
    size_t size;
};

TMFile TMFileOpen(AAssetManager  *assetManager, const char *filepath);
void TMFileClose(TMFile *file);


#endif //MY_APPLICATION_TM_FILE_H
