#ifndef NX_IMPORT_ANIMATION_IMPORTER_HPP
#define NX_IMPORT_ANIMATION_IMPORTER_HPP

#include <NX/NX_Render.h>

#include "../INX_GlobalPool.hpp"
#include "./SceneImporter.hpp"
#include "./AssimpHelper.hpp"
#include "NX/NX_Model.h"
#include <SDL3/SDL_assert.h>

namespace import {

/* === Declaration === */

class AnimationImporter {
public:
    /** Constructors */
    AnimationImporter(const SceneImporter& importer);

    /** Loads all animations contained in the imported scene */
    NX_Animation** loadAnimations(int* animCount, int targetFrameRate);

private:
    /** Load an animation */
    bool loadAnimation(NX_Animation* animation, const aiAnimation* aiAnim, int targetFrameRate);

    /** Calculate a pose recursively */
    void getPoseRecursive(
        const aiNode* node, const aiAnimation* anim, float time, const NX_Mat4& parentMatrix,
        NX_Mat4* globalMatrices, NX_Transform* localTransforms,
        const NX_BoneInfo* bones, int totalBones);

    /** Interpolation */
    bool getNodeTransformAtTime(NX_Transform* outTransform, const aiAnimation* anim, const char* nodeName, float time);
    NX_Vec3 interpolateKeyFrames(const aiVectorKey* keys, uint32_t numKeys, float time);
    NX_Quat interpolateKeyFrames(const aiQuatKey* keys, uint32_t numKeys, float time);

private:
    const SceneImporter& mImporter;
};

/* === Public Implementation === */

inline AnimationImporter::AnimationImporter(const SceneImporter& importer)
    : mImporter(importer)
{
    SDL_assert(importer.isValid());
}

inline NX_Animation** AnimationImporter::loadAnimations(int* animCount, int targetFrameRate)
{
    *animCount = 0;

    if (mImporter.animationCount() == 0) {
        NX_LOG(E, "RENDER: No animations found");
        return nullptr;
    }

    NX_Animation** animations = static_cast<NX_Animation**>(SDL_calloc(
        mImporter.animationCount(), sizeof(NX_Animation*)
    ));

    if (animations == nullptr) {
        NX_LOG(E, "RENDER: Unable to allocate memory for animations");
        return nullptr;
    }

    size_t successCount = 0;
    for (uint32_t i = 0; i < mImporter.animationCount(); i++) {
        animations[successCount] = INX_Pool.Create<NX_Animation>();
        const aiAnimation* aiAnim = mImporter.animation(i);
        if (loadAnimation(animations[successCount], aiAnim, targetFrameRate)) {
            successCount++;
        }
        else {
            NX_LOG(E, "RENDER: Failed to process animation %d", i);
        }
    }

    if (successCount == 0) {
        NX_LOG(E, "RENDER: No animations were successfully loaded");
        SDL_free(animations);
        return nullptr;
    }

    if (successCount < mImporter.animationCount()) {
        NX_LOG(W, "RENDER: Only %d out of %d animations were successfully loaded", successCount, mImporter.animationCount());
        NX_Animation** resizedAnims = static_cast<NX_Animation**>(SDL_realloc(animations, successCount * sizeof(NX_Animation*)));
        if (resizedAnims) animations = resizedAnims;
    }

    *animCount = successCount;

    return animations;
}

/* === Private Implementation === */

inline bool AnimationImporter::loadAnimation(NX_Animation* animation, const aiAnimation* aiAnim, int targetFrameRate)
{
    /* --- Initialize animation name --- */

    SDL_strlcpy(animation->name, aiAnim->mName.data, sizeof(animation->name));

    /* --- Compute frame count --- */

    float ticksPerSecond = aiAnim->mTicksPerSecond ? aiAnim->mTicksPerSecond : 25.0f;
    float durationInSeconds = (float)aiAnim->mDuration / ticksPerSecond;
    animation->frameCount = (int)(durationInSeconds * targetFrameRate + 0.5f);

    /* --- Count unique bones --- */

    int boneCounter = 0;
    for (uint32_t meshIndex = 0; meshIndex < mImporter.meshCount(); meshIndex++) {
        const aiMesh* mesh = mImporter.mesh(meshIndex);
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
            const aiBone* bone = mesh->mBones[boneIndex];
            bool exists = false;
            // Check in previous meshes
            for (uint32_t pm = 0; pm < meshIndex && !exists; pm++) {
                const aiMesh* prevMesh = mImporter.mesh(pm);
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
        NX_LOG(W, "RENDER: No bones found for animation '%s'", animation->name);
        return false;
    }

    animation->boneCount = boneCounter;

    /* --- Allocate storage --- */

    animation->bones = static_cast<NX_BoneInfo*>(SDL_calloc(animation->boneCount, sizeof(NX_BoneInfo)));
    animation->frameGlobalPoses = static_cast<NX_Mat4**>(SDL_calloc(animation->frameCount, sizeof(NX_Mat4*)));
    animation->frameLocalPoses = static_cast<NX_Transform**>(SDL_calloc(animation->frameCount, sizeof(NX_Transform*)));

    if (!animation->bones || !animation->frameGlobalPoses || !animation->frameLocalPoses) {
        NX_LOG(E, "RENDER: Allocation failed");
        SDL_free(animation->bones);
        SDL_free(animation->frameGlobalPoses);
        SDL_free(animation->frameLocalPoses);
        return false;
    }

    /* --- Collect unique bone names --- */

    boneCounter = 0;
    for (uint32_t meshIndex = 0; meshIndex < mImporter.meshCount(); meshIndex++) {
        const aiMesh* mesh = mImporter.mesh(meshIndex);
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
        animation->frameGlobalPoses[f] = static_cast<NX_Mat4*>(SDL_calloc(animation->boneCount, sizeof(NX_Mat4)));
        animation->frameLocalPoses[f] = static_cast<NX_Transform*>(SDL_calloc(animation->boneCount, sizeof(NX_Transform)));
        if (!animation->frameGlobalPoses[f] || !animation->frameLocalPoses[f]) {
            NX_LOG(E, "RENDER: Failed to allocate frame %d", f);
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
            animation->frameGlobalPoses[f][b] = NX_MAT4_IDENTITY;
            animation->frameLocalPoses[f][b] = NX_TRANSFORM_IDENTITY;
        }

        getPoseRecursive(
            mImporter.rootNode(), aiAnim, timeInTicks,
            NX_MAT4_IDENTITY,
            animation->frameGlobalPoses[f],
            animation->frameLocalPoses[f],
            animation->bones, animation->boneCount
        );
    }

    return true;
}

inline void AnimationImporter::getPoseRecursive(
    const aiNode* node, const aiAnimation* anim, float time, const NX_Mat4& parentMatrix,
    NX_Mat4* globalMatrices, NX_Transform* localTransforms,
    const NX_BoneInfo* bones, int totalBones)
{
    NX_Transform transform = NX_TRANSFORM_IDENTITY;
    NX_Mat4 matrix = NX_MAT4_IDENTITY;

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
        matrix = NX_TransformToMat4(&transform);
    }
    else {
        // No animation for this node, use its default bind transform
        transform = NX_Mat4Decompose(&matrix);
        matrix = AssimpCast<NX_Mat4>(node->mTransformation);
    }

    /* --- Convert local transform to global by multiplying with parent's global matrix --- */

    matrix = NX_Mat4Mul(&matrix, &parentMatrix);

    /* --- Store global matrix and local transform for this bone (if it exists) --- */

    if (boneIndex >= 0) {
        globalMatrices[boneIndex] = matrix;      // Global transform of the bone in world/model space
        localTransforms[boneIndex] = transform;  // Local transform relative to parent
    }

    /* --- Recursively process all child nodes to propagate transforms through hierarchy --- */

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        getPoseRecursive(
            node->mChildren[i], anim, time, matrix,
            globalMatrices, localTransforms, bones, totalBones
        );
    }
}

inline bool AnimationImporter::getNodeTransformAtTime(NX_Transform* outTransform, const aiAnimation* anim, const char* nodeName, float time)
{
    SDL_assert(outTransform && anim && nodeName);

    /* --- Search for the animation channel corresponding to the given node --- */

    for (uint32_t i = 0; i < anim->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = anim->mChannels[i];
        if (SDL_strcmp(nodeAnim->mNodeName.data, nodeName) == 0)
        {
            outTransform->translation = interpolateKeyFrames(
                nodeAnim->mPositionKeys, nodeAnim->mNumPositionKeys, time
            );
            outTransform->rotation = interpolateKeyFrames(
                nodeAnim->mRotationKeys, nodeAnim->mNumRotationKeys, time
            );
            outTransform->scale = interpolateKeyFrames(
                nodeAnim->mScalingKeys, nodeAnim->mNumScalingKeys, time
            );
            return true;
        }
    }

    /* --- No animation channel found for the node: return identity matrix --- */

    return false;
}

inline NX_Vec3 AnimationImporter::interpolateKeyFrames(const aiVectorKey* keys, uint32_t numKeys, float time)
{
    /* --- Case where there is only one key --- */

    if (numKeys == 1) {
        return AssimpCast<NX_Vec3>(keys[0].mValue);
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
        return AssimpCast<NX_Vec3>(keys[numKeys - 1].mValue);
    }

    /* --- Interpolate using Assimp::Interpolator --- */

    float deltaTime = keys[index + 1].mTime - keys[index].mTime;
    float factor = (time - keys[index].mTime) / deltaTime;

    aiVector3D result;
    Assimp::Interpolator<aiVectorKey>()(result, keys[index], keys[index + 1], factor);

    return AssimpCast<NX_Vec3>(result);
}

inline NX_Quat AnimationImporter::interpolateKeyFrames(const aiQuatKey* keys, uint32_t numKeys, float time)
{
    /* --- Case where there is only one key --- */

    if (numKeys == 1) {
        return AssimpCast<NX_Quat>(keys[0].mValue);
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
        return AssimpCast<NX_Quat>(keys[numKeys - 1].mValue);
    }

    /* --- Interpolate using Assimp::Interpolator --- */

    float deltaTime = keys[index + 1].mTime - keys[index].mTime;
    float factor = (time - keys[index].mTime) / deltaTime;

    aiQuaternion result;
    Assimp::Interpolator<aiQuatKey>()(result, keys[index], keys[index + 1], factor);

    return AssimpCast<NX_Quat>(result);
}

} // namespace import

#endif // NX_IMPORT_ANIMATION_IMPORTER_HPP
