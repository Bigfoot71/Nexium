/* NX_Animation.cpp -- API definition for Nexium's animation module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Animation.h>

#include <NX/NX_Filesystem.h>
#include <NX/NX_Memory.h>

#include "./Importer/AnimationImporter.hpp"

// ============================================================================
// PUBLIC API
// ============================================================================

NX_AnimationLib* NX_LoadAnimationLib(const char* filePath, int targetFrameRate)
{
    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);

    NX_AnimationLib* animLib = NX_LoadAnimationLibFromData(
        fileData, fileSize, INX_GetFileExt(filePath), targetFrameRate
    );

    NX_Free(fileData);

    return animLib;
}

NX_AnimationLib* NX_LoadAnimationLibFromData(const void* data, unsigned int size, const char* hint, int targetFrameRate)
{
    import::SceneImporter importer(data, size, hint);
    if (!importer.isValid()) {
        return nullptr;
    }

    return import::AnimationImporter(importer).LoadAnimationLib(targetFrameRate);
}

void NX_DestroyAnimationLib(NX_AnimationLib* animLib)
{
    if (animLib == nullptr) {
        return;
    }

    for (int i = 0; i < animLib->count; i++) {
        NX_Animation& anim = animLib->animations[i];
        for (int j = 0; j < anim.frameCount; j++) {
            NX_Free(anim.frameGlobalPoses[j]);
            NX_Free(anim.frameLocalPoses[j]);
        }
        NX_Free(anim.frameGlobalPoses);
        NX_Free(anim.frameLocalPoses);
        NX_Free(anim.bones);
    }

    INX_Pool.Destroy(animLib);
}

NX_Animation* NX_GetAnimation(const NX_AnimationLib* animLib, const char* name)
{
    for (int i = 0; i < animLib->count; i++) {
        if (SDL_strcmp(animLib->animations[i].name, name) == 0) {
            return &animLib->animations[i];
        }
    }
    return nullptr;
}
