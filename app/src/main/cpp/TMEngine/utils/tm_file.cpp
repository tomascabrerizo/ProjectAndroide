//
// Created by Manuel Cabrerizo on 26/02/2023.
//

#include "tm_file.h"

#include <android/log.h>

#include <stdlib.h>

#define TM_LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "App2", __VA_ARGS__))

TMFile TMFileOpen(AAssetManager *assetManager, const char *filepath) {
    TMFile result{};
    AAsset *file = AAssetManager_open(assetManager, filepath, AASSET_MODE_BUFFER);
    if(!file) {
        TM_LOG_INFO("Error Loading file: %s", filepath);
        return result;
    }

    long fileSize = AAsset_getLength(file);
    result.data = malloc(fileSize + 1);
    result.size = fileSize;
    AAsset_read (file,result.data,fileSize);
    char *buffer = (char *)result.data;
    buffer[fileSize] = 0; // null terminating string...
    AAsset_close(file);

    return result;
}

void TMFileClose(TMFile *file) {
    if(file->data) free(file->data);
    file->data = NULL;
    file->size = 0;
}