/* PoolModelAnimation.cpp -- Contains the implementation for model animation loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "../PoolModel.hpp"
#include "./AssimpHelper.hpp"

#include <assimp/anim.h>

/* === Helpers === */

namespace {

HP_Vec3 interpolateAnimationKeysVec3(const aiVectorKey* keys, uint32_t numKeys, float time)
{
    /* --- Case where there is only one key --- */

    if (numKeys == 1) {
        return assimp_cast<HP_Vec3>(keys[0].mValue);
    }

    /* --- Find surrounding keys --- */

    uint32_t index = 0;
    for (uint32_t i = 0; i < numKeys - 1; i++) {
        if (time < keys[i + 1].mTime) {
            index = i;
            break;
        }
    }

    /* --- Clamp to last key --- */

    if (index >= numKeys - 1) {
        return assimp_cast<HP_Vec3>(keys[numKeys - 1].mValue);
    }

    /* --- Interpolate using Assimp::Interpolator --- */

    float deltaTime = keys[index + 1].mTime - keys[index].mTime;
    float factor = (time - keys[index].mTime) / deltaTime;

    aiVector3D result;
    Assimp::Interpolator<aiVectorKey>()(result, keys[index], keys[index + 1], factor);

    return assimp_cast<HP_Vec3>(result);
}

HP_Quat interpolateAnimationKeysQuat(const aiQuatKey* keys, uint32_t numKeys, float time)
{
    /* --- Case where there is only one key --- */

    if (numKeys == 1) {
        return assimp_cast<HP_Quat>(keys[0].mValue);
    }

    /* --- Find surrounding keys --- */

    uint32_t index = 0;
    for (uint32_t i = 0; i < numKeys - 1; i++) {
        if (time < keys[i + 1].mTime) {
            index = i;
            break;
        }
    }

    /* --- Clamp to last key --- */

    if (index >= numKeys - 1) {
        return assimp_cast<HP_Quat>(keys[numKeys - 1].mValue);
    }

    /* --- Interpolate using Assimp::Interpolator --- */

    float deltaTime = keys[index + 1].mTime - keys[index].mTime;
    float factor = (time - keys[index].mTime) / deltaTime;

    aiQuaternion result;
    Assimp::Interpolator<aiQuatKey>()(result, keys[index], keys[index + 1], factor);

    return assimp_cast<HP_Quat>(result);
}

bool getNodeTransformAtTime(HP_Transform* outTransform, const aiAnimation* anim, const char* nodeName, float time)
{
    SDL_assert(outTransform && anim && nodeName);

    /* --- Search for the animation channel corresponding to the given node --- */

    for (uint32_t i = 0; i < anim->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = anim->mChannels[i];
        if (SDL_strcmp(nodeAnim->mNodeName.data, nodeName) == 0)
        {
            outTransform->translation = interpolateAnimationKeysVec3(
                nodeAnim->mPositionKeys, nodeAnim->mNumPositionKeys, time
            );
            outTransform->rotation = interpolateAnimationKeysQuat(
                nodeAnim->mRotationKeys, nodeAnim->mNumRotationKeys, time
            );
            outTransform->scale = interpolateAnimationKeysVec3(
                nodeAnim->mScalingKeys, nodeAnim->mNumScalingKeys, time
            );
            return true;
        }
    }

    /* --- No animation channel found for the node: return identity matrix --- */

    return false;
}

void calculateAnimationTransforms(
    const aiNode* node, const aiAnimation* anim, float time, const HP_Mat4& parentMatrix,
    HP_Mat4* globalMatrices, HP_Transform* localTransforms,
    const HP_BoneInfo* bones, int totalBones)
{
    HP_Transform transform = HP_TRANSFORM_IDENTITY;
    HP_Mat4 matrix = HP_MAT4_IDENTITY;

    /* --- Find the corresponding bone index for this node (if it exists) --- */

    int boneIndex = -1;
    for (int i = 0; i < totalBones; i++) {
        if (SDL_strcmp(node->mName.data, bones[i].name) == 0) {
            boneIndex = i;
            break;
        }
    }

    /* --- Get the node's local transform at the specified time from the animation --- */

    if (getNodeTransformAtTime(&transform, anim, node->mName.data, time)) {
        matrix = HP_TransformToMat4(&transform);
    }
    else {
        // No animation for this node, use its default bind transform
        transform = HP_Mat4Decompose(&matrix);
        matrix = assimp_cast<HP_Mat4>(node->mTransformation);
    }

    /* --- Convert local transform to global by multiplying with parent's global matrix --- */

    matrix = HP_Mat4Mul(&matrix, &parentMatrix);

    /* --- Store global matrix and local transform for this bone (if it exists) --- */

    if (boneIndex >= 0) {
        globalMatrices[boneIndex] = matrix;      // Global transform of the bone in world/model space
        localTransforms[boneIndex] = transform;  // Local transform relative to parent
    }

    /* --- Recursively process all child nodes to propagate transforms through hierarchy --- */

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        calculateAnimationTransforms(
            node->mChildren[i], anim, time, matrix,
            globalMatrices, localTransforms, bones, totalBones
        );
    }
}

} // namespace

/* === Implementation === */

namespace render {

bool PoolModel::processAnimation(HP_ModelAnimation* animation, const aiScene* scene, const aiAnimation* aiAnim, int targetFrameRate)
{
    /* --- Validate input --- */

    if (!animation || !scene || !aiAnim) {
        return false;
    }

    /* --- Initialize animation name --- */

    SDL_strlcpy(animation->name, aiAnim->mName.data, sizeof(animation->name));

    /* --- Compute frame count --- */

    float ticksPerSecond = aiAnim->mTicksPerSecond ? aiAnim->mTicksPerSecond : 25.0f;
    float durationInSeconds = (float)aiAnim->mDuration / ticksPerSecond;
    animation->frameCount = (int)(durationInSeconds * targetFrameRate + 0.5f);

    /* --- Count unique bones --- */

    int boneCounter = 0;
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
            const aiBone* bone = mesh->mBones[boneIndex];
            bool exists = false;
            // Check in previous meshes
            for (uint32_t pm = 0; pm < meshIndex && !exists; pm++) {
                const aiMesh* prevMesh = scene->mMeshes[pm];
                for (uint32_t pb = 0; pb < prevMesh->mNumBones && !exists; pb++) {
                    exists = (SDL_strcmp(bone->mName.data, prevMesh->mBones[pb]->mName.data) == 0);
                }
            }
            // Check in current mesh (previous bones)
            if (!exists) {
                for (uint32_t pb = 0; pb < boneIndex && !exists; pb++) {
                    exists = (SDL_strcmp(bone->mName.data, mesh->mBones[pb]->mName.data) == 0);
                }
            }
            if (!exists) boneCounter++;
        }
    }

    if (boneCounter == 0) {
        HP_INTERNAL_LOG(W, "RENDER: No bones found for animation '%s'", animation->name);
        return false;
    }

    animation->boneCount = boneCounter;

    /* --- Allocate storage --- */

    animation->bones = static_cast<HP_BoneInfo*>(SDL_calloc(animation->boneCount, sizeof(HP_BoneInfo)));
    animation->frameGlobalPoses = static_cast<HP_Mat4**>(SDL_calloc(animation->frameCount, sizeof(HP_Mat4*)));
    animation->frameLocalPoses = static_cast<HP_Transform**>(SDL_calloc(animation->frameCount, sizeof(HP_Transform*)));

    if (!animation->bones || !animation->frameGlobalPoses || !animation->frameLocalPoses) {
        HP_INTERNAL_LOG(E, "RENDER: Allocation failed");
        SDL_free(animation->bones);
        SDL_free(animation->frameGlobalPoses);
        SDL_free(animation->frameLocalPoses);
        return false;
    }

    /* --- Collect unique bone names --- */

    boneCounter = 0;
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
            const aiBone* bone = mesh->mBones[boneIndex];
            bool exists = false;
            for (int i = 0; i < boneCounter && !exists; i++) {
                exists = (SDL_strcmp(bone->mName.data, animation->bones[i].name) == 0);
            }
            if (!exists) {
                SDL_strlcpy(animation->bones[boneCounter].name, bone->mName.data, sizeof(animation->bones[boneCounter].name));
                animation->bones[boneCounter].parent = -1;
                boneCounter++;
            }
        }
    }

    /* --- Allocate per-frame storage --- */

    for (int f = 0; f < animation->frameCount; f++) {
        animation->frameGlobalPoses[f] = static_cast<HP_Mat4*>(SDL_calloc(animation->boneCount, sizeof(HP_Mat4)));
        animation->frameLocalPoses[f] = static_cast<HP_Transform*>(SDL_calloc(animation->boneCount, sizeof(HP_Transform)));
        if (!animation->frameGlobalPoses[f] || !animation->frameLocalPoses[f]) {
            HP_INTERNAL_LOG(E, "RENDER: Failed to allocate frame %d", f);
            for (int i = 0; i <= f; i++) {
                SDL_free(animation->frameGlobalPoses[i]);
                SDL_free(animation->frameLocalPoses[i]);
            }
            SDL_free(animation->frameGlobalPoses);
            SDL_free(animation->frameLocalPoses);
            SDL_free(animation->bones);
            return false;
        }
    }

    /* --- Compute transforms for all frames in a single pass --- */

    for (int f = 0; f < animation->frameCount; f++)
    {
        float timeInTicks = fminf(((float)f / targetFrameRate) * ticksPerSecond, (float)aiAnim->mDuration);

        for (int b = 0; b < animation->boneCount; b++) {
            animation->frameGlobalPoses[f][b] = HP_MAT4_IDENTITY;
            animation->frameLocalPoses[f][b] = HP_TRANSFORM_IDENTITY;
        }

        calculateAnimationTransforms(
            scene->mRootNode, aiAnim, timeInTicks,
            HP_MAT4_IDENTITY,
            animation->frameGlobalPoses[f],
            animation->frameLocalPoses[f],
            animation->bones, animation->boneCount
        );
    }

    return true;
}

} // namespace render
