/* NX_Skeleton.cpp -- API definition for Nexium's skeleton module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Skeleton.h>

#include <NX/NX_Filesystem.h>
#include <NX/NX_Memory.h>

#include "./Importer/SkeletonImporter.hpp"

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Skeleton* NX_LoadSkeleton(const char* filePath)
{
    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);

    NX_Skeleton* skeleton = NX_LoadSkeletonFromData(
        fileData, fileSize, INX_GetFileExt(filePath)
    );

    NX_Free(fileData);

    return skeleton;
}

NX_Skeleton* NX_LoadSkeletonFromData(const void* data, unsigned int size, const char* hint)
{
    import::SceneImporter importer(data, size, hint);
    if (!importer.IsValid()) {
        return nullptr;
    }

    return import::SkeletonImporter(importer).ProcessSkeleton();
}

void NX_DestroySkeleton(NX_Skeleton* skeleton)
{
    if (skeleton == nullptr) {
        return;
    }

    NX_Free(skeleton->boneOffsets);
    NX_Free(skeleton->bindLocal);
    NX_Free(skeleton->bindPose);
    NX_Free(skeleton->bones);

    INX_Pool.Destroy(skeleton);
}
