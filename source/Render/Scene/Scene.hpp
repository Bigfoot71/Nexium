/* Scene.hpp -- Scene system management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_SCENE_HPP
#define HP_SCENE_HPP

#include <Hyperion/HP_Render.h>

#include "../../Detail/Util/BucketArray.hpp"
#include "../../Detail/GPU/Framebuffer.hpp"
#include "../../Detail/GPU/SwapBuffer.hpp"
#include "../../Detail/GPU/MipBuffer.hpp"
#include "../../Detail/GPU/Texture.hpp"

#include "../Core/ProgramCache.hpp"
#include "../Core/AssetCache.hpp"

#include "../HP_RenderTexture.hpp"
#include "./RenderableBuffer.hpp"
#include "./MaterialBuffer.hpp"
#include "./LightManager.hpp"
#include "./Environment.hpp"
#include "./ViewFrustum.hpp"
#include "./BoneBuffer.hpp"
#include "./DrawCall.hpp"
#include "./DrawData.hpp"

namespace scene {

/* === Declaration === */

class Scene {
public:
    Scene(render::ProgramCache& programs, render::AssetCache& assets, HP_AppDesc& desc);

    /** Begin/End 3D mode functions */
    void begin(const HP_Camera& camera, const HP_Environment& env, const HP_RenderTexture* target);
    void end();

    /** Push draw call functions */
    void drawMesh(const HP_Mesh& mesh, const HP_InstanceBuffer* instances, int instanceCount, const HP_Material& material, const HP_Transform& transform);
    void drawModel(const HP_Model& model, const HP_InstanceBuffer* instances, int instanceCount, const HP_Transform& transform);

    const LightManager& lights() const;
    LightManager& lights();

private:
    /** Scene rendering functions */
    void renderBackground(const gpu::Pipeline& pipeline);
    void renderPrePass(const gpu::Pipeline& pipeline);
    void renderScene(const gpu::Pipeline& pipeline);

    /** Post process functions */
    const gpu::Texture& postSSAO(const gpu::Texture& source);
    const gpu::Texture& postBloom(const gpu::Texture& source);
    void postFinal(const gpu::Texture& source);

private:
    struct TargetInfo {
        const HP_RenderTexture* target{nullptr};
        HP_IVec2 resolution{};
        float aspect{};
    };

private:
    /** Shared assets */
    render::ProgramCache& mPrograms;
    render::AssetCache& mAssets;

    /** Draw calls */
    BucketDrawCalls mDrawCalls{};
    ArrayDrawData mDrawData{};

    /** Scene data */
    Environment mEnvironment{};
    ViewFrustum mFrustum{};

    /** Managers */
    RenderableBuffer mRenderableBuffer;
    MaterialBuffer mMaterialBuffer;
    BoneBuffer mBoneBuffer;
    LightManager mLights;

    /** Scene render targets */
    gpu::Texture mTargetSceneColor{};
    gpu::Texture mTargetSceneNormal{};
    gpu::Texture mTargetSceneDepth{};
    gpu::Framebuffer mFramebufferScene{};

    /** Mip chain */
    gpu::MipBuffer mMipChain{};

    /** Swap buffers */
    gpu::SwapBuffer mSwapPostProcess{};     //< Ping-pong buffer used during scene post process
    gpu::SwapBuffer mSwapAuxiliary{};       //< Secondary ping-pong buffer in half resolution

    /** State infos */
    TargetInfo mTargetInfo{};
};

/* === Public Implementation === */

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
