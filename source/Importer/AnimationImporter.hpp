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
    NX_AnimationLib* LoadAnimationLib();

private:
    /** Load an animation */
    bool LoadAnimation(NX_Animation* animation, const aiAnimation* aiAnim);

    /** Load a single animation channel */
    bool LoadChannel(NX_AnimationChannel* channel, const aiNodeAnim* aiChannel);

private:
    const SceneImporter& mImporter;
};

/* === Public Implementation === */

inline AnimationImporter::AnimationImporter(const SceneImporter& importer)
    : mImporter(importer)
{
    SDL_assert(importer.IsValid());
}

inline NX_AnimationLib* AnimationImporter::LoadAnimationLib()
{
    if (mImporter.GetAnimationCount() == 0) {
        NX_LOG(E, "RENDER: No animations found");
        return nullptr;
    }

    NX_Animation* animations = NX_Calloc<NX_Animation>(mImporter.GetAnimationCount());
    if (!animations) {
        NX_LOG(E, "RENDER: Unable to allocate memory for animations");
        return nullptr;
    }

    size_t successCount = 0;
    for (uint32_t i = 0; i < mImporter.GetAnimationCount(); i++) {
        if (LoadAnimation(&animations[successCount], mImporter.GetAnimation(i))) {
            successCount++;
        } else {
            NX_LOG(E, "RENDER: Failed to process animation %d", i);
        }
    }

    if (successCount == 0) {
        NX_LOG(E, "RENDER: No animations were successfully loaded");
        NX_Free(animations);
        return nullptr;
    }

    if (successCount < mImporter.GetAnimationCount()) {
        NX_LOG(W, "RENDER: Only %zu out of %u animations were successfully loaded", successCount, mImporter.GetAnimationCount());
        NX_Animation* resizedAnims = NX_Realloc<NX_Animation>(animations, successCount);
        if (resizedAnims) animations = resizedAnims;
    }

    NX_AnimationLib* animLib = INX_Pool.Create<NX_AnimationLib>();
    animLib->animations = animations;
    animLib->count = successCount;

    return animLib;
}

/* === Private Implementation === */

inline bool AnimationImporter::LoadAnimation(NX_Animation* animation, const aiAnimation* aiAnim)
{
    /* --- Basic validation --- */

    if (!aiAnim || aiAnim->mNumChannels == 0) {
        NX_LOG(E, "RENDER: Invalid animation or no channels");
        return false;
    }

    /* --- Retrieving the number of bones from the imported skeleton --- */

    const int boneCount = mImporter.GetBoneCount();
    if (boneCount == 0) {
        NX_LOG(E, "RENDER: No bones in skeleton");
        return false;
    }

    /* --- Initializing the NX_Animation structure --- */

    animation->boneCount = boneCount;
    animation->duration = static_cast<float>(aiAnim->mDuration);
    animation->ticksPerSecond = aiAnim->mTicksPerSecond != 0.0f 
        ? static_cast<float>(aiAnim->mTicksPerSecond) 
        : 24.0f;

    /* --- Copy of the animation name --- */

    const size_t nameLen = std::min(strlen(aiAnim->mName.C_Str()), sizeof(animation->name) - 1);
    SDL_memcpy(animation->name, aiAnim->mName.C_Str(), nameLen);
    animation->name[nameLen] = '\0';

    /* --- Channel allocation --- */

    animation->channelCount = aiAnim->mNumChannels;
    animation->channels = NX_Calloc<NX_AnimationChannel>(animation->channelCount);
    if (!animation->channels) {
        NX_LOG(E, "RENDER: Failed to allocate animation channels");
        return false;
    }

    /* --- Loading each channel --- */

    uint32_t successChannels = 0;
    for (uint32_t i = 0; i < aiAnim->mNumChannels; i++) {
        if (LoadChannel(&animation->channels[successChannels], aiAnim->mChannels[i])) {
            successChannels++;
        } else {
            NX_LOG(W, "RENDER: Failed to load channel %u", i);
        }
    }

    if (successChannels == 0) {
        NX_LOG(E, "RENDER: No channels were successfully loaded");
        NX_Free(animation->channels);
        return false;
    }

    /* --- Adjust the number of channels if some have failed --- */

    if (successChannels < animation->channelCount) {
        animation->channelCount = successChannels;
        NX_AnimationChannel* resized = NX_Realloc<NX_AnimationChannel>(
            animation->channels, successChannels);
        if (resized) animation->channels = resized;
    }

    NX_LOG(V, "RENDER: Animation '%s' loaded: %.2f duration, %.2f ticks/sec, %u channels", 
           animation->name, animation->duration, animation->ticksPerSecond, 
           animation->channelCount);

    return true;
}

inline bool AnimationImporter::LoadChannel(NX_AnimationChannel* channel, const aiNodeAnim* aiChannel)
{
    if (!aiChannel) {
        NX_LOG(E, "RENDER: Invalid animation channel");
        return false;
    }

    const char* boneName = aiChannel->mNodeName.C_Str();
    channel->boneIndex = mImporter.GetBoneIndex(boneName);
    if (channel->boneIndex < 0) {
        NX_LOG(W, "RENDER: Bone '%s' from animation not found in skeleton", boneName);
        return false;
    }

    channel->positionKeyCount = aiChannel->mNumPositionKeys;
    if (channel->positionKeyCount > 0) {
        channel->positionKeys = NX_Malloc<NX_Vec3Key>(channel->positionKeyCount);
        if (!channel->positionKeys) {
            NX_LOG(E, "RENDER: Failed to allocate position keys");
            return false;
        }
        for (uint32_t i = 0; i < channel->positionKeyCount; i++) {
            channel->positionKeys[i].time = static_cast<float>(aiChannel->mPositionKeys[i].mTime);
            channel->positionKeys[i].value = AssimpCast<NX_Vec3>(aiChannel->mPositionKeys[i].mValue);
        }
    }
    else {
        channel->positionKeys = nullptr;
    }

    channel->rotationKeyCount = aiChannel->mNumRotationKeys;
    if (channel->rotationKeyCount > 0) {
        channel->rotationKeys = NX_Malloc<NX_QuatKey>(channel->rotationKeyCount);
        if (!channel->rotationKeys) {
            NX_LOG(E, "RENDER: Failed to allocate rotation keys");
            NX_Free(channel->positionKeys);
            return false;
        }
        for (uint32_t i = 0; i < channel->rotationKeyCount; i++) {
            channel->rotationKeys[i].time = static_cast<float>(aiChannel->mRotationKeys[i].mTime);
            channel->rotationKeys[i].value = AssimpCast<NX_Quat>(aiChannel->mRotationKeys[i].mValue);
        }
    }
    else {
        channel->rotationKeys = nullptr;
    }

    channel->scaleKeyCount = aiChannel->mNumScalingKeys;
    if (channel->scaleKeyCount > 0) {
        channel->scaleKeys = NX_Malloc<NX_Vec3Key>(channel->scaleKeyCount);
        if (!channel->scaleKeys) {
            NX_LOG(E, "RENDER: Failed to allocate scale keys");
            NX_Free(channel->positionKeys);
            NX_Free(channel->rotationKeys);
            return false;
        }
        for (uint32_t i = 0; i < channel->scaleKeyCount; i++) {
            channel->scaleKeys[i].time = static_cast<float>(aiChannel->mScalingKeys[i].mTime);
            channel->scaleKeys[i].value = AssimpCast<NX_Vec3>(aiChannel->mScalingKeys[i].mValue);
        }
    }
    else {
        channel->scaleKeys = nullptr;
    }

    return true;
}

} // namespace import

#endif // NX_IMPORT_ANIMATION_IMPORTER_HPP
