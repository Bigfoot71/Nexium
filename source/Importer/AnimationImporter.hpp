#ifndef NX_IMPORT_ANIMATION_IMPORTER_HPP
#define NX_IMPORT_ANIMATION_IMPORTER_HPP

#include "../INX_GlobalPool.hpp"
#include "./SceneImporter.hpp"
#include "./AssimpHelper.hpp"
#include <SDL3/SDL_assert.h>

namespace import {

/* === Declaration === */

class AnimationImporter {
public:
    /** Constructors */
    AnimationImporter(const SceneImporter& importer);

    /** Loads all animations contained in the imported scene */
    NX_AnimationLib* LoadAnimationLib(int targetFrameRate);

private:
    /** Load an animation */
    bool LoadAnimation(NX_Animation* animation, const aiAnimation* aiAnim, int targetFrameRate);

    /** Calculate a pose recursively */
    void GetPoseRecursive(
        const aiNode* node, const aiAnimation* anim, float time, const NX_Mat4& parentMatrix,
        NX_Mat4* globalMatrices, NX_Transform* localTransforms,
        const NX_BoneInfo* bones, int totalBones);

    /** Interpolation */
    bool GetNodeTransformAtTime(NX_Transform* outTransform, const aiAnimation* anim, const char* nodeName, float time);
    NX_Vec3 InterpolateKeyFrames(const aiVectorKey* keys, uint32_t numKeys, float time);
    NX_Quat InterpolateKeyFrames(const aiQuatKey* keys, uint32_t numKeys, float time);

private:
    const SceneImporter& mImporter;
};

/* === Public Implementation === */

inline AnimationImporter::AnimationImporter(const SceneImporter& importer)
    : mImporter(importer)
{
    SDL_assert(importer.isValid());
}

inline NX_AnimationLib* AnimationImporter::LoadAnimationLib(int targetFrameRate)
{
    if (mImporter.animationCount() == 0) {
        NX_LOG(E, "RENDER: No animations found");
        return nullptr;
    }

    NX_Animation* animations = NX_Calloc<NX_Animation>(mImporter.animationCount());
    if (animations == nullptr) {
        NX_LOG(E, "RENDER: Unable to allocate memory for animations");
        return nullptr;
    }

    size_t successCount = 0;
    for (uint32_t i = 0; i < mImporter.animationCount(); i++) {
        const aiAnimation* aiAnim = mImporter.animation(i);
        if (LoadAnimation(&animations[successCount], aiAnim, targetFrameRate)) {
            successCount++;
        }
        else {
            NX_LOG(E, "RENDER: Failed to process animation %d", i);
        }
    }

    if (successCount == 0) {
        NX_LOG(E, "RENDER: No animations were successfully loaded");
        NX_Free(animations);
        return nullptr;
    }

    if (successCount < mImporter.animationCount()) {
        NX_LOG(W, "RENDER: Only %d out of %d animations were successfully loaded", successCount, mImporter.animationCount());
        NX_Animation* resizedAnims = NX_Realloc<NX_Animation>(animations, successCount);
        if (resizedAnims) animations = resizedAnims;
    }

    NX_AnimationLib* animLib = INX_Pool.Create<NX_AnimationLib>();
    animLib->animations = animations;
    animLib->count = successCount;

    return animLib;
}

/* === Private Implementation === */

inline bool AnimationImporter::LoadAnimation(NX_Animation* animation, const aiAnimation* aiAnim, int targetFrameRate)
{
    /* --- Initialize animation name --- */

    SDL_strlcpy(animation->name, aiAnim->mName.data, sizeof(animation->name));

    /* --- Compute frame count --- */

    float ticksPerSecond = aiAnim->mTicksPerSecond ? aiAnim->mTicksPerSecond : 25.0f;
    float durationInSeconds = static_cast<float>(aiAnim->mDuration) / ticksPerSecond;
    animation->frameCount = static_cast<int>(durationInSeconds * targetFrameRate + 0.5f);

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

    animation->bones = NX_Calloc<NX_BoneInfo>(animation->boneCount);
    animation->frameGlobalPoses = NX_Calloc<NX_Mat4*>(animation->frameCount);
    animation->frameLocalPoses = NX_Calloc<NX_Transform*>(animation->frameCount);

    if (!animation->bones || !animation->frameGlobalPoses || !animation->frameLocalPoses) {
        NX_LOG(E, "RENDER: Allocation failed");
        NX_Free(animation->bones);
        NX_Free(animation->frameGlobalPoses);
        NX_Free(animation->frameLocalPoses);
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
        animation->frameGlobalPoses[f] = NX_Calloc<NX_Mat4>(animation->boneCount);
        animation->frameLocalPoses[f] = NX_Calloc<NX_Transform>(animation->boneCount);
        if (!animation->frameGlobalPoses[f] || !animation->frameLocalPoses[f]) {
            NX_LOG(E, "RENDER: Failed to allocate frame %d", f);
            for (int i = 0; i <= f; i++) {
                NX_Free(animation->frameGlobalPoses[i]);
                NX_Free(animation->frameLocalPoses[i]);
            }
            NX_Free(animation->frameGlobalPoses);
            NX_Free(animation->frameLocalPoses);
            NX_Free(animation->bones);
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

        GetPoseRecursive(
            mImporter.rootNode(), aiAnim, timeInTicks,
            NX_MAT4_IDENTITY,
            animation->frameGlobalPoses[f],
            animation->frameLocalPoses[f],
            animation->bones, animation->boneCount
        );
    }

    return true;
}

inline void AnimationImporter::GetPoseRecursive(
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

    if (GetNodeTransformAtTime(&transform, anim, node->mName.data, time)) {
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
        GetPoseRecursive(
            node->mChildren[i], anim, time, matrix,
            globalMatrices, localTransforms, bones, totalBones
        );
    }
}

inline bool AnimationImporter::GetNodeTransformAtTime(NX_Transform* outTransform, const aiAnimation* anim, const char* nodeName, float time)
{
    SDL_assert(outTransform && anim && nodeName);

    /* --- Search for the animation channel corresponding to the given node --- */

    for (uint32_t i = 0; i < anim->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = anim->mChannels[i];
        if (SDL_strcmp(nodeAnim->mNodeName.data, nodeName) == 0)
        {
            outTransform->translation = InterpolateKeyFrames(
                nodeAnim->mPositionKeys, nodeAnim->mNumPositionKeys, time
            );
            outTransform->rotation = InterpolateKeyFrames(
                nodeAnim->mRotationKeys, nodeAnim->mNumRotationKeys, time
            );
            outTransform->scale = InterpolateKeyFrames(
                nodeAnim->mScalingKeys, nodeAnim->mNumScalingKeys, time
            );
            return true;
        }
    }

    /* --- No animation channel found for the node: return identity matrix --- */

    return false;
}

inline NX_Vec3 AnimationImporter::InterpolateKeyFrames(const aiVectorKey* keys, uint32_t numKeys, float time)
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

inline NX_Quat AnimationImporter::InterpolateKeyFrames(const aiQuatKey* keys, uint32_t numKeys, float time)
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
