#include <NX/NX_AnimationPlayer.h>
#include <NX/NX_Animation.h>
#include <NX/NX_Math.h>

#include "./INX_GlobalPool.hpp"

// ============================================================================
// INTERNAL INTERPOLATION FUNCTIONS
// ============================================================================

template <typename T>
void INX_FindKeyFrames(const T* keys, uint32_t keyCount, float time,
                       uint32_t* outIdx0, uint32_t* outIdx1, float* outT)
{
    if (keyCount == 0) {
        *outIdx0 = *outIdx1 = 0;
        *outT = 0.0f;
        return;
    }

    if (keyCount == 1 || time <= keys[0].time) {
        *outIdx0 = *outIdx1 = 0;
        *outT = 0.0f;
        return;
    }

    if (time >= keys[keyCount - 1].time) {
        *outIdx0 = *outIdx1 = keyCount - 1;
        *outT = 0.0f;
        return;
    }

    uint32_t left = 0;
    uint32_t right = keyCount - 1;

    while (right - left > 1) {
        uint32_t mid = (left + right) / 2;
        if (keys[mid].time <= time) left = mid;
        else right = mid;
    }

    *outIdx0 = left;
    *outIdx1 = right;

    float t0 = keys[*outIdx0].time;
    float t1 = keys[*outIdx1].time;
    float delta = t1 - t0;

    *outT = (delta > 0.0f) ? (time - t0) / delta : 0.0f;
}

static NX_Transform INX_InterpolateChannel(const NX_AnimationChannel* channel, float time)
{
    NX_Transform result = NX_TRANSFORM_IDENTITY;

    if (channel->positionKeyCount > 0) {
        uint32_t idx0, idx1;
        float t;
        INX_FindKeyFrames(channel->positionKeys, channel->positionKeyCount, time, &idx0, &idx1, &t);
        const NX_Vec3& v0 = channel->positionKeys[idx0].value;
        const NX_Vec3& v1 = channel->positionKeys[idx1].value;
        result.translation = NX_Vec3Lerp(v0, v1, t);
    }

    if (channel->rotationKeyCount > 0) {
        uint32_t idx0, idx1;
        float t;
        INX_FindKeyFrames(channel->rotationKeys, channel->rotationKeyCount, time, &idx0, &idx1, &t);
        const NX_Quat& q0 = channel->rotationKeys[idx0].value;
        const NX_Quat& q1 = channel->rotationKeys[idx1].value;
        result.rotation = NX_QuatSLerp(q0, q1, t);
    }

    if (channel->scaleKeyCount > 0) {
        uint32_t idx0, idx1;
        float t;
        INX_FindKeyFrames(channel->scaleKeys, channel->scaleKeyCount, time, &idx0, &idx1, &t);
        const NX_Vec3& s0 = channel->scaleKeys[idx0].value;
        const NX_Vec3& s1 = channel->scaleKeys[idx1].value;
        result.scale = NX_Vec3Lerp(s0, s1, t);
    }

    return result;
}

// ============================================================================
// INTERNAL POSE COMPUTATION
// ============================================================================

static const NX_AnimationChannel* INX_FindChannelForBone(const NX_Animation& anim, int iBone)
{
    for (int i = 0; i < anim.channelCount; i++) {
        if (anim.channels[i].boneIndex == iBone) {
            return &anim.channels[i];
        }
    }
    return nullptr;
}

static void INX_ComputePose(NX_AnimationPlayer& player, float totalWeight)
{
    const int boneCount = player.skeleton->boneCount;
    const int animCount = player.animLib->count;
    NX_AnimationState* states = player.states;

    for (int iBone = 0; iBone < boneCount; iBone++)
    {
        NX_Transform blended{};
        bool isAnimated{false};

        for (int iAnim = 0; iAnim < animCount; iAnim++)
        {
            const NX_Animation& anim = player.animLib->animations[iAnim];
            const NX_AnimationState& state = states[iAnim];
            if (state.weight <= 0.0f) continue;

            const NX_AnimationChannel* channel = INX_FindChannelForBone(anim, iBone);
            if (!channel) continue;
            isAnimated = true;

            NX_Transform local = INX_InterpolateChannel(channel, state.currentTime * anim.ticksPerSecond);
            float w = state.weight / totalWeight;

            blended.translation += local.translation * w;
            blended.rotation += local.rotation * w;
            blended.scale += local.scale * w;
        }

        if (isAnimated) {
            blended.rotation = NX_QuatNormalize(blended.rotation);
            player.currentPose[iBone] = NX_TransformToMat4(&blended);
        }
        else {
            player.currentPose[iBone] = player.skeleton->bindLocal[iBone];
        }

        int parentIdx = player.skeleton->bones[iBone].parent;
        if (parentIdx >= 0) {
            player.currentPose[iBone] = NX_Mat4Mul(&player.currentPose[iBone], &player.currentPose[parentIdx]);
        }
        else {
            NX_Mat4 invLocalBind = NX_Mat4Inverse(&player.skeleton->bindLocal[iBone]);
            NX_Mat4 parentGlobalScene = NX_Mat4Mul(&invLocalBind, &player.skeleton->bindPose[iBone]);
            player.currentPose[iBone] = NX_Mat4Mul(&player.currentPose[iBone], &parentGlobalScene);
        }
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_AnimationPlayer* NX_CreateAnimationPlayer(const NX_Skeleton* skeleton, const NX_AnimationLib* animLib)
{
    NX_AnimationPlayer* player = INX_Pool.Create<NX_AnimationPlayer>();

    player->skeleton = skeleton;
    player->animLib = animLib;

    player->states = NX_Calloc<NX_AnimationState>(animLib->count);
    player->currentPose = NX_Calloc<NX_Mat4>(skeleton->boneCount);

    return player;
}

void NX_DestroyAnimationPlayer(NX_AnimationPlayer* player)
{
    NX_Free(player->currentPose);
    NX_Free(player->states);

    INX_Pool.Destroy(player);
}

void NX_UpdateAnimationPlayer(NX_AnimationPlayer* player, float dt)
{
    const int boneCount = player->skeleton->boneCount;
    const int animCount = player->animLib->count;

    NX_AnimationState* states = player->states;
    NX_Mat4* pose = player->currentPose;

    float totalWeight = 0.0f;
    for (int iAnim = 0; iAnim < animCount; iAnim++) {
        totalWeight += states[iAnim].weight;
    }

    if (totalWeight <= 0.0f) {
        SDL_memcpy(pose, player->skeleton->bindPose, boneCount * sizeof(NX_Mat4));
    }
    else {
        INX_ComputePose(*player, totalWeight);
    }

    for (int iAnim = 0; iAnim < animCount; iAnim++)
    {
        const NX_Animation& anim = player->animLib->animations[iAnim];
        NX_AnimationState& state = states[iAnim];

        state.currentTime += dt;

        float durationInSeconds = anim.duration / anim.ticksPerSecond;

        if (state.currentTime >= durationInSeconds) {
            state.currentTime = state.loop
                ? std::fmod(state.currentTime, durationInSeconds)
                : durationInSeconds;
        }
    }
}
