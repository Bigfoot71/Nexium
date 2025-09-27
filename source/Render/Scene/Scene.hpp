/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef HP_SCENE_HPP
#define HP_SCENE_HPP

#include <Hyperion/HP_Render.h>

#include "../../Detail/Util/BucketArray.hpp"
#include "../../Detail/GPU/Framebuffer.hpp"
#include "../../Detail/GPU/SwapBuffer.hpp"
#include "../../Detail/GPU/Texture.hpp"
#include "../Core/SharedAssets.hpp"

#include "./BoneBufferManager.hpp"
#include "./SharedAssets.hpp"
#include "./ProgramCache.hpp"
#include "./LightManager.hpp"
#include "./ViewFrustum.hpp"
#include "./DrawCall.hpp"
#include "./DrawData.hpp"

namespace scene {

/* === Declaration === */

class Scene {
public:
    Scene(const render::SharedAssets& assets, HP_AppDesc& desc);

    /** Begin/End 3D mode functions */
    void begin(const HP_Camera& camera, const HP_Environment& env);
    void end();

    /** Push draw call functions */
    void drawMesh(const HP_Mesh& mesh, const HP_InstanceBuffer* instances, int instanceCount, const HP_Material& material, const HP_Transform& transform);
    void drawModel(const HP_Model& model, const HP_InstanceBuffer* instances, int instanceCount, const HP_Transform& transform);

    ProgramCache& programs();
    const LightManager& lights() const;
    LightManager& lights();

private:
    void renderScene();
    void postSSAO(bool firstPass);
    void postFinal(bool firstPass);

private:
    struct TargetInfo {
        HP_IVec2 resolution;
        HP_Vec2 texelSize;
        float aspect;
    };

private:
    /** Shared assets */
    const render::SharedAssets& mAssetsCommon;
    scene::SharedAssets mAssetsScene;

    /** Scene data */
    HP_Environment mEnvironment{};
    BucketDrawCalls mDrawCalls{};
    ArrayDrawData mDrawData{};
    ViewFrustum mFrustum{};

    /** Managers */
    BoneBufferManager mBoneBuffer;
    ProgramCache mPrograms;
    LightManager mLights;

    /** Scene render targets */
    gpu::Texture mTargetSceneColor{};
    gpu::Texture mTargetSceneNormal{};
    gpu::Texture mTargetSceneDepth{};
    gpu::Framebuffer mFramebufferScene{};

    /** Swap buffers */
    gpu::SwapBuffer mSwapPostProcess{};     //< Ping-pong buffer used during scene post process
    gpu::SwapBuffer mSwapAuxiliary{};       //< Secondary ping-pong buffer in half resolution

    /** State infos */
    TargetInfo mTargetInfo{};
};

/* === Public Implementation === */

inline void Scene::begin(const HP_Camera& camera, const HP_Environment& env)
{
    mFrustum.update(camera, mTargetInfo.aspect);
    mEnvironment = env;
}

inline void Scene::drawMesh(const HP_Mesh& mesh, const HP_InstanceBuffer* instances, int instanceCount, const HP_Material& material, const HP_Transform& transform)
{
    int dataIndex = mDrawData.size();
    mDrawData.emplace_back(transform, instances, instanceCount);
    mDrawCalls.emplace(DrawCall::category(material), dataIndex, mesh, material);
}

inline void Scene::drawModel(const HP_Model& model, const HP_InstanceBuffer* instances, int instanceCount, const HP_Transform& transform)
{
    /* --- If the model is rigged we upload its bone transformations to the buffer --- */

    int boneMatrixOffset = -1;

    if (model.boneCount > 0)
    {
        const HP_Mat4* boneMatrices = model.boneBindPose;

        if (model.animMode == HP_ANIM_INTERNAL && model.anim != nullptr) {
            if (model.boneCount != model.anim->boneCount) {
                HP_INTERNAL_LOG(W, "RENDER: Model and animation bone counts differ");
            }
            int frame = static_cast<int>(model.animFrame + 0.5) % model.anim->frameCount;
            boneMatrices = model.anim->frameGlobalPoses[frame];
        }
        else if (model.animMode == HP_ANIM_CUSTOM && model.boneOverride != nullptr) {
            boneMatrices = model.boneOverride;
        }

        boneMatrixOffset = mBoneBuffer.upload(
            model.boneOffsets,
            boneMatrices,
            model.boneCount
        );
    }

    /* --- Adding shared meshes and data to the batch --- */

    int dataIndex = mDrawData.size();
    for (int i = 0; i < model.meshCount; i++) {
        const HP_Material& material = model.materials[model.meshMaterials[i]];
        mDrawCalls.emplace(DrawCall::category(material), dataIndex, *model.meshes[i], material);
    }

    mDrawData.emplace_back(transform, instances, instanceCount, boneMatrixOffset);
}

inline void Scene::end()
{
    /* --- Sort draw calls --- */

    mDrawCalls.sort(DrawCall::Category::TRANSPARENT,
        [this](const DrawCall& a, const DrawCall& b) {
            float maxDistA = mFrustum.getDistanceSquaredToFarthestPoint(a.mesh().aabb, mDrawData[a.dataIndex()].matrix());
            float maxDistB = mFrustum.getDistanceSquaredToFarthestPoint(b.mesh().aabb, mDrawData[b.dataIndex()].matrix());
            return maxDistA > maxDistB;
        }
    );

    /* --- Process lights --- */

    mLights.process({
        .programs = mPrograms,
        .viewFrustum = mFrustum,
        .textureWhite = mAssetsCommon.textureWhite().gpuTexture(),
        .environement = mEnvironment,
        .boneBuffer = mBoneBuffer,
        .drawCalls = mDrawCalls,
        .drawData = mDrawData
    });

    /* --- Render scene --- */

    renderScene();

    bool firstPass = true;

    if (mEnvironment.ssao.enabled) {
        postSSAO(firstPass);
        firstPass = false;
    }

    postFinal(firstPass);

    /* --- Reset state --- */

    mBoneBuffer.clear();
    mDrawCalls.clear();
    mDrawData.clear();
}

inline ProgramCache& Scene::programs()
{
    return mPrograms;
}

inline const LightManager& Scene::lights() const
{
    return mLights;
}

inline LightManager& Scene::lights()
{
    return mLights;
}

} // namespace scene

#endif // HP_SCENE_HPP
