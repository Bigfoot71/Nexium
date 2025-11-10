/* NX_Render.cpp -- API definition for Nexium's render module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Environment.h>
#include <NX/NX_Render3D.h>
#include <NX/NX_BitUtils.h>
#include <NX/NX_Runtime.h>
#include <NX/NX_Texture.h>
#include <NX/NX_Display.h>
#include <NX/NX_Window.h>
#include <NX/NX_Camera.h>
#include <NX/NX_Init.h>
#include <NX/NX_Font.h>
#include <NX/NX_Math.h>
#include <NX/NX_Log.h>

#include "./NX_InstanceBuffer.hpp"
#include "./NX_RenderTexture.hpp"
#include "./NX_Shader3D.hpp"
#include "./NX_Texture.hpp"
#include "./NX_Light.hpp"
#include "./NX_Shape.hpp"

#include "./Detail/Util/DynamicArray.hpp"
#include "./Detail/Util/BucketArray.hpp"
#include "./Detail/Util/Ranges.hpp"

#include "./Detail/GPU/StagingBuffer.hpp"
#include "./Detail/GPU/Framebuffer.hpp"
#include "./Detail/GPU/SwapBuffer.hpp"
#include "./Detail/GPU/MipBuffer.hpp"
#include "./Detail/GPU/Texture.hpp"
#include "./Detail/GPU/Buffer.hpp"

#include "./INX_GPUProgramCache.hpp"
#include "./INX_GlobalAssets.hpp"
#include "./INX_VariantMesh.hpp"
#include "./INX_GlobalPool.hpp"
#include "./INX_GPUBridge.hpp"
#include "./INX_Frustum.hpp"

#include <numeric>

// ============================================================================
// INTERNAL ENUMS
// ============================================================================

enum class INX_RenderPass {
    RENDER_NONE,            //< Value indicating that no rendering pass is in progress
    RENDER_SCENE,           //< Value indicating that a scene rendering pass is in progress
    RENDER_SHADOW,          //< Value indicating that a shadow rendering pass is in progress
    RENDER_PASS_COUNT
};

enum INX_DrawType : uint8_t {
    DRAW_OPAQUE = 0,        //< Represents all purely opaque objects
    DRAW_PREPASS = 1,       //< Represents objects rendered with a depth pre-pass (can be opaque or transparent)
    DRAW_TRANSPARENT = 2,   //< Represents all transparent objects
    DRAW_TYPE_COUNT
};

// ============================================================================
// INTERNAL STRUCTS
// ============================================================================

/** Main scene view frustum data */
struct INX_ViewFrustum : public INX_Frustum {
    NX_Mat4 viewProj;
    NX_Mat4 view;
    NX_Mat4 proj;
    NX_Mat4 invViewProj;
    NX_Mat4 invView;
    NX_Mat4 invProj;
    NX_Vec3 position;
    NX_Layer cullMask;
    float near;
    float far;
};

/** Shared CPU data per draw call */
struct INX_DrawShared {
    /** Spatial data */
    NX_Transform transform;
    INX_BoundingSphere3D sphere;
    /** Instances data */
    const NX_InstanceBuffer* instances;
    int instanceCount;
    /** Animations */
    int boneMatrixOffset;                   //< If less than zero, no animation assigned
    /** Unique data */
    int uniqueDataIndex;
    int uniqueDataCount;
};

/** Unique CPU data per draw call */
struct INX_DrawUnique {
    /** Object to draw */
    INX_VariantMesh mesh;
    NX_Material material;
    INX_OrientedBoundingBox3D obb;
    /** Additionnal data */
    NX_Shader3D::TextureArray textures;     //< Array containing the textures linked to the material shader at the time of draw (if any)
    int dynamicRangeIndex;                  //< Index of the material shader's dynamic uniform buffer range (if any)
    /** Shared/Unique data */
    int sharedDataIndex;                    //< Index to the shared data that this unique draw call data depends on
    int uniqueDataIndex;                    //< Is actually the index of INX_DrawUnique itself, useful when iterating through sorted categories
    /** Object type */
    INX_DrawType type;
};

/** Data for active lights */
struct INX_ActiveLight {
    NX_Light* light;
    int32_t shadowIndex;
};

// ============================================================================
// INTERNAL GPU STRUCTS
// ============================================================================

/** Uniform frustum data */
struct INX_GPUFrustum {
    alignas(16) NX_Mat4 viewProj;
    alignas(16) NX_Mat4 view;
    alignas(16) NX_Mat4 proj;
    alignas(16) NX_Mat4 invViewProj;
    alignas(16) NX_Mat4 invView;
    alignas(16) NX_Mat4 invProj;
    alignas(16) NX_Vec3 position;
    alignas(4) float near;
    alignas(4) float far;
};

/** Per-frame data for scene rendering */
struct INX_GPUSceneFrame {
    alignas(8) NX_IVec2 screenSize;
    alignas(16) NX_IVec3 clusterCount;
    alignas(4) uint32_t maxLightsPerCluster;
    alignas(4) float clusterSliceScale;
    alignas(4) float clusterSliceBias;
    alignas(4) float elapsedTime;
    alignas(4) int32_t hasActiveLights;
    alignas(4) int32_t hasProbe;
};

/** Per-frame data for shadow maps rendering */
struct INX_GPUShadowFrame {
    alignas(16) NX_Mat4 lightViewProj;
    alignas(16) NX_Mat4 cameraInvView;
    alignas(16) NX_Vec3 lightPosition;
    alignas(4) float lightRange;
    alignas(4) int32_t lightType;
    alignas(4) float elapsedTime;
};

/** Uniform environment data */
struct INX_GPUEnvironment {
    alignas(16) NX_Vec3 ambientColor;
    alignas(16) NX_Vec4 skyRotation;
    alignas(16) NX_Vec3 fogColor;
    alignas(16) NX_Vec4 bloomPrefilter;
    alignas(4) float skyIntensity;
    alignas(4) float skySpecular;
    alignas(4) float skyDiffuse;
    alignas(4) float fogDensity;
    alignas(4) float fogStart;
    alignas(4) float fogEnd;
    alignas(4) float fogSkyAffect;
    alignas(4) int fogMode;
    alignas(4) float ssaoIntensity;
    alignas(4) float ssaoRadius;
    alignas(4) float ssaoPower;
    alignas(4) float ssaoBias;
    alignas(4) int ssaoEnabled;
    alignas(4) float bloomFilterRadius;
    alignas(4) float bloomStrength;
    alignas(4) int bloomMode;
    alignas(4) float adjustBrightness;
    alignas(4) float adjustContrast;
    alignas(4) float adjustSaturation;
    alignas(4) float tonemapExposure;
    alignas(4) float tonemapWhite;
    alignas(4) int tonemapMode;
};

/** Shared GPU data per draw call */
struct INX_GPUDrawShared {
    alignas(16) NX_Mat4 matModel;
    alignas(16) NX_Mat4 matNormal;
    alignas(4) int32_t boneOffset;
    alignas(4) int32_t instancing;
    alignas(4) int32_t skinning;
};

/** Unique GPU data per draw call */
struct INX_GPUDrawUnique {
    alignas(16) NX_Vec4 albedoColor;
    alignas(16) NX_Vec3 emissionColor;
    alignas(4) float emissionEnergy;
    alignas(4) float aoLightAffect;
    alignas(4) float occlusion;
    alignas(4) float roughness;
    alignas(4) float metalness;
    alignas(4) float normalScale;
    alignas(4) float alphaCutOff;
    alignas(4) float depthOffset;
    alignas(4) float depthScale;
    alignas(8) NX_Vec2 texOffset;
    alignas(8) NX_Vec2 texScale;
    alignas(4) int32_t billboard;
    alignas(4) uint32_t layerMask;
};

// ============================================================================
// LOCAL STATE
// ============================================================================

struct INX_SceneState {
    /** Environment */
    NX_Color background{};
    NX_Cubemap* skyCubemap{};
    NX_ReflectionProbe* skyProbe{};

    /** Frustum data */
    INX_ViewFrustum viewFrustum{};

    /** Scene render targets */
    gpu::Texture targetSceneColor{};        //< RGBA16F
    gpu::Texture targetSceneNormal{};       //< RGB8
    gpu::Texture targetSceneDepth{};        //< D24
    gpu::Framebuffer framebufferScene{};

    /** Additionnal framebuffers */
    gpu::SwapBuffer swapPostProcess{};      //< Ping-pong buffer used during scene post process
    gpu::SwapBuffer swapAuxiliary{};        //< Secondary ping-pong buffer in half resolution
    gpu::MipBuffer mipChain{};              //< Mipchain, primarily used for down/up sampling the bloom

    /** Post processing data */
    util::DynamicArray<float> bloomLevels{};
    NX_Tonemap tonemapMode{};
    NX_Bloom bloomMode{};
    bool ssaoEnabled{};

    /** Uniform buffers */
    gpu::Buffer frustumUniform{};
    gpu::Buffer frameUniform{};
    gpu::Buffer envUniform{};

    /** Target info */
    const NX_RenderTexture* target{nullptr};
    NX_IVec2 targetResolution{};
    float targetAspect{};
};

struct INX_LightingState {
    /** Aliases */
    using ActiveLights = util::DynamicArray<INX_ActiveLight>;
    using ActiveShadows = util::BucketArray<NX_Light*, NX_LightType, NX_LIGHT_TYPE_COUNT>;
    using ShadowsNeedingUpdate = util::BucketArray<uint32_t, NX_LightType, NX_LIGHT_TYPE_COUNT>; 

    /** Constants */
    static constexpr float SlicesPerDepthOctave = 3.0f;     ///< Number of depth slices per depth octave
    static constexpr uint32_t MaxLightsPerCluster = 32;     ///< Maximum number of lights in a single cluster

    /** Storage buffers */
    gpu::Buffer storageLights{};        ///< Active lights (sorted DIR -> SPOT -> OMNI)
    gpu::Buffer storageShadow{};        ///< Per-light shadow data
    gpu::Buffer storageClusters{};      ///< Per-cluster light counts (numDir, numSpot, numOmni)
    gpu::Buffer storageIndices{};       ///< Per-cluster light indices (grouped by type)
    gpu::Buffer storageClusterAABB{};   ///< Per-cluster AABBs (computed during culling)

    /** Per-frame caches */
    ActiveLights activeLights{};        ///< Active lights (pointers + shadow indices), same order as storageLights
    ActiveShadows activeShadows{};      ///< Active shadow-casting lights, bucketed by type, same order as storageShadow

    /** Additionnal Data */
    NX_IVec3 clusterCount{};            ///< Number of clusters X/Y/Z
    NX_IVec2 clusterSize{};             ///< Size of a cluster X/Y
    float clusterSliceScale{};
    float clusterSliceBias{};
};

struct INX_ShadowingState {
    /** Shadow framebuffers and targets (one per light type) */
    std::array<util::DynamicArray<bool>, NX_LIGHT_TYPE_COUNT> assigned{};   ///< Contains bools indicating whether the shadow map is used by a light.
    std::array<gpu::Framebuffer, NX_LIGHT_TYPE_COUNT> framebuffer{};        ///< Contains framebuffers per light type
    std::array<gpu::Texture, NX_LIGHT_TYPE_COUNT> target{};                 ///< Contains textures arrays per light type (cubemap for omni-lights)
    gpu::Texture targetDepth{};                                             ///< Common depth buffer for depth testing (TODO: Make it a renderbuffer)

    /** Uniform buffers */
    gpu::Buffer frameUniform{};

    /** Current light caster target (during shadow map pass) */
    NX_Light* casterTarget{};
    NX_Camera mainCamera{};
};

struct INX_DrawCallState {
    /** Draw call data stored in RAM */
    util::DynamicArray<INX_DrawShared> sharedData{};
    util::DynamicArray<INX_DrawUnique> uniqueData{};

    /** Sorted draw call array */
    util::BucketArray<int, INX_DrawType, DRAW_TYPE_COUNT> uniqueVisible{};

    /** Sorting cache */
    util::DynamicArray<float> sortDistances{};

    /** Draw call data stored in VRAM */
    gpu::StagingBuffer<NX_Mat4> boneBuffer{};
    gpu::Buffer sharedBuffer{};
    gpu::Buffer uniqueBuffer{};
};

struct INX_Render3DState {
    /** Scene state */
    INX_SceneState scene{};

    /** Lighting state */
    INX_LightingState lighting{};
    INX_ShadowingState shadowing{};

    /** Common state infos */
    INX_DrawCallState drawCalls{};
    NX_RenderFlags renderFlags{};
    INX_RenderPass renderPass{};
};

static util::UniquePtr<INX_Render3DState> INX_Render3D{};

// ============================================================================
// LOCAL HELPER FUNCTIONS
// ============================================================================

static const char* INX_GetRenderPassName(INX_RenderPass renderPass)
{
    static constexpr const char* names[int(INX_RenderPass::RENDER_PASS_COUNT)] = {
        "None", "Scene", "Shadow"
    };

    return names[int(renderPass)];
}

// ============================================================================
// LOCAL INIT FUNCTIONS
// ============================================================================

static void INX_UpdateAppDesc(NX_AppDesc* desc)
{
    if (desc->render3D.resolution < NX_IVEC2_ONE) {
        desc->render3D.resolution = NX_GetDisplaySize();
    }

    if (desc->render3D.shadowRes < 1) {
        desc->render3D.shadowRes = 2048;
    }

    desc->render3D.sampleCount = NX_MAX(desc->render3D.sampleCount, 1);
}

static void INX_InitSceneState(INX_SceneState* scene, const NX_AppDesc* desc)
{
    /* --- Create scene render targets --- */

    scene->targetSceneColor = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGBA16F,
            .data = nullptr,
            .width = desc->render3D.resolution.x,
            .height = desc->render3D.resolution.y
        }
    );

    scene->targetSceneNormal = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RG8,
            .data = nullptr,
            .width = desc->render3D.resolution.x,
            .height = desc->render3D.resolution.y
        }
    );

    scene->targetSceneDepth = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_DEPTH_COMPONENT24,
            .data = nullptr,
            .width = desc->render3D.resolution.x,
            .height = desc->render3D.resolution.y
        }
    );

    /* --- Configure scene framebuffer --- */

    scene->framebufferScene = gpu::Framebuffer(
        { &scene->targetSceneColor, &scene->targetSceneNormal },
        &scene->targetSceneDepth
    );

    if (desc->render3D.sampleCount > 1) {
        scene->framebufferScene.SetSampleCount(desc->render3D.sampleCount);
    }

    /* --- Create swap buffers --- */

    scene->swapPostProcess = gpu::SwapBuffer(
        desc->render3D.resolution.x,
        desc->render3D.resolution.y,
        GL_RGB16F
    );

    scene->swapAuxiliary = gpu::SwapBuffer(
        desc->render3D.resolution.x / 2,
        desc->render3D.resolution.y / 2,
        GL_RGB16F
    );

    /* --- Create mip chain --- */

    scene->mipChain = gpu::MipBuffer(
        desc->render3D.resolution.x / 2,
        desc->render3D.resolution.y / 2,
        GL_RGB16F
    );

    /* --- Create uniform buffers --- */

    scene->frustumUniform = gpu::Buffer(
        GL_UNIFORM_BUFFER, sizeof(INX_GPUFrustum),
        nullptr, GL_DYNAMIC_DRAW
    );

    scene->frameUniform = gpu::Buffer(
        GL_UNIFORM_BUFFER, sizeof(INX_GPUSceneFrame),
        nullptr, GL_DYNAMIC_DRAW
    );

    scene->envUniform = gpu::Buffer(
        GL_UNIFORM_BUFFER, sizeof(INX_GPUEnvironment),
        nullptr, GL_DYNAMIC_DRAW
    );
}

static void INX_InitLightingState(INX_LightingState* lighting, const NX_AppDesc* desc)
{
    /* --- Calculating the size and number of clusters for lighting --- */

    // NOTE: The Z dimension defined here is the minimum number of slices allocated initially.
    //       During rendering, the actual Z slices are dynamic and calculated per-frame based on
    //       the camera's near and far planes using a logarithmic distribution.

    const NX_IVec2& resolution = desc->render3D.resolution;

    lighting->clusterSize.x = std::max(16, resolution.x / 80); // 80 px per target cluster
    lighting->clusterSize.y = std::max(9, resolution.y / 50);  // 50 px per target cluster

    lighting->clusterCount.x = NX_DIV_CEIL(resolution.x, lighting->clusterSize.x);
    lighting->clusterCount.y = NX_DIV_CEIL(resolution.y, lighting->clusterSize.y);
    lighting->clusterCount.z = 16;

    int clusterTotal = lighting->clusterCount.x *
                       lighting->clusterCount.y *
                       lighting->clusterCount.z;

    /* --- Create light and shadow storages --- */

    lighting->storageLights = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        32 * sizeof(INX_GPULight),
        nullptr, GL_DYNAMIC_DRAW
    );

    lighting->storageShadow = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        32 * sizeof(INX_GPUShadow),
        nullptr, GL_DYNAMIC_DRAW
    );

    /* --- Create lihgting cluster storages --- */

    lighting->storageClusters = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        clusterTotal * 4 * sizeof(uint32_t),
        nullptr, GL_DYNAMIC_COPY
    );

    lighting->storageIndices = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        clusterTotal * lighting->MaxLightsPerCluster * sizeof(uint32_t),
        nullptr, GL_DYNAMIC_COPY
    );

    lighting->storageClusterAABB = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        clusterTotal * sizeof(NX_BoundingBox3D),
        nullptr, GL_DYNAMIC_COPY
    );

    /* --- Reserve light caches space --- */

    if (!lighting->activeLights.Reserve(32)) {
        NX_LOG(E, "RENDER: Active lights cache pre-allocation failed (requested: 32 entries)");
    }

    if (!lighting->activeShadows.Reserve(8)) {
        NX_LOG(E, "RENDER: Active shadows cache pre-allocation failed (requested: 8 entries)");
    }
}

static void INX_InitShadowState(INX_ShadowingState* shadowing, const NX_AppDesc* desc)
{
    /* --- Init used shadow array --- */

    if (!shadowing->assigned[NX_LIGHT_DIR].Resize(8)) {
        NX_LOG(E, "RENDER: Failed to pre-allocate directional shadow map assignement cache (requested 8 entries)");
    }

    if (!shadowing->assigned[NX_LIGHT_SPOT].Resize(8)) {
        NX_LOG(E, "RENDER: Failed to pre-allocate spot shadow map assignement cache (requested 8 entries)");
    }

    if (!shadowing->assigned[NX_LIGHT_OMNI].Resize(8)) {
        NX_LOG(E, "RENDER: Failed to pre-allocate omni shadow map assignement cache (requested 8 entries)");
    }

    /* --- Create shadow maps --- */

    shadowing->target[NX_LIGHT_DIR] = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D_ARRAY,
            .internalFormat = GL_R16F,
            .width = desc->render3D.shadowRes,
            .height = desc->render3D.shadowRes,
            .depth = 1,
            .mipmap = false
        }
    );

    shadowing->target[NX_LIGHT_SPOT] = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D_ARRAY,
            .internalFormat = GL_R16F,
            .width = desc->render3D.shadowRes,
            .height = desc->render3D.shadowRes,
            .depth = 1,
            .mipmap = false
        }
    );

    shadowing->target[NX_LIGHT_OMNI] = gpu::Texture(
        gpu::TextureConfig {
            .target = GL_TEXTURE_CUBE_MAP_ARRAY,
            .internalFormat = GL_R16F,
            .width = desc->render3D.shadowRes,
            .height = desc->render3D.shadowRes,
            .depth = 1,
            .mipmap = false
        }
    );

    shadowing->targetDepth = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_DEPTH_COMPONENT24,
            .width = desc->render3D.shadowRes,
            .height = desc->render3D.shadowRes,
            .mipmap = false,
        }
    );

    /* --- Create shadow map framebuffers --- */

    for (int i = 0; i < shadowing->framebuffer.size(); i++) {
        shadowing->framebuffer[i] = gpu::Framebuffer(
            {&shadowing->target[i]},
            &shadowing->targetDepth
        );
    }

    /* --- Create frame uniform buffer --- */

    shadowing->frameUniform = gpu::Buffer(
        GL_UNIFORM_BUFFER, sizeof(INX_GPUShadowFrame),
        nullptr, GL_DYNAMIC_DRAW
    );
}

static void INX_InitDrawCallState(INX_DrawCallState* drawCalls)
{
    constexpr int drawCallReserveCount = 1024;

    drawCalls->sharedBuffer = gpu::Buffer(GL_SHADER_STORAGE_BUFFER, drawCallReserveCount * sizeof(INX_GPUDrawShared));
    drawCalls->uniqueBuffer = gpu::Buffer(GL_SHADER_STORAGE_BUFFER, drawCallReserveCount * sizeof(INX_GPUDrawUnique));
    drawCalls->boneBuffer = gpu::StagingBuffer<NX_Mat4>(GL_SHADER_STORAGE_BUFFER, 1024);

    if (!drawCalls->sharedData.Reserve(drawCallReserveCount)) {
        NX_LOG(E, "RENDER: Shared draw call data array pre-allocation failed (requested: %i entries)", drawCallReserveCount);
    }

    if (!drawCalls->uniqueData.Reserve(drawCallReserveCount)) {
        NX_LOG(E, "RENDER: Unique draw call data array pre-allocation failed (requested: %i entries)", drawCallReserveCount);
    }

    if (!drawCalls->uniqueVisible.Reserve(drawCallReserveCount)) {
        NX_LOG(E, "RENDER: Visible unique draw call list pre-allocation failed (requested: %i entries)", drawCallReserveCount);
    }
}

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

bool INX_Render3DState_Init(NX_AppDesc* desc)
{
    INX_Render3D = util::MakeUnique<INX_Render3DState>();
    if (INX_Render3D == nullptr) {
        return false;
    }

    INX_UpdateAppDesc(desc);

    INX_InitSceneState(&INX_Render3D->scene, desc);
    INX_InitLightingState(&INX_Render3D->lighting, desc);
    INX_InitShadowState(&INX_Render3D->shadowing, desc);
    INX_InitDrawCallState(&INX_Render3D->drawCalls);

    return true;
}

void INX_Render3DState_Quit()
{
    INX_Render3D.reset();
}

int INX_Render3DState_RequestShadowMap(NX_LightType type)
{
    INX_ShadowingState& shadowing = INX_Render3D->shadowing;

    util::DynamicArray<bool>& usageCache = shadowing.assigned[type];
    gpu::Framebuffer& shadowFb = shadowing.framebuffer[type];
    gpu::Texture& shadowMap = shadowing.target[type];

    int mapIndex = 0;

    while (true) {
        if (mapIndex > usageCache.GetSize()) {
            usageCache.Resize(2 * usageCache.GetSize());
        }
        if (!usageCache[mapIndex]) {
            usageCache[mapIndex] = true;
            break;
        }
    }

    if (mapIndex > shadowMap.GetDepth()) {
        shadowMap.Realloc(shadowMap.GetWidth(), shadowMap.GetHeight(), mapIndex);
        shadowFb.UpdateColorTextureView(0, shadowMap);
    }

    return mapIndex;
}

void INX_Render3DState_ReleaseShadowMap(NX_LightType type, int mapIndex)
{
    INX_Render3D->shadowing.assigned[type][mapIndex] = false;
}

// ============================================================================
// LOCAL FUNCTIONS
// ============================================================================

static bool INX_BeginRenderPass(INX_RenderPass pass, NX_RenderFlags flags)
{
    if (INX_Render3D->renderPass != INX_RenderPass::RENDER_NONE) {
        const char* request = INX_GetRenderPassName(pass);
        const char* current = INX_GetRenderPassName(INX_Render3D->renderPass);
        NX_LOG(W,"RENDER: Cannot begin render pass (%s); Another render pass is already active (%s)", request, current);
        return false;
    }

    INX_Render3D->renderFlags = flags;
    INX_Render3D->renderPass = pass;

    return true;
}

static bool INX_CheckRenderPass(INX_RenderPass pass)
{
    if (INX_Render3D->renderPass != pass) {
        const char* request = INX_GetRenderPassName(pass);
        const char* current = INX_GetRenderPassName(INX_Render3D->renderPass);
        NX_LOG(W, "RENDER: Cannot end render pass (%s); Current render pass is (%s)", request, current);
        return false;
    }

    return true;
}

static void INX_EndRenderPass()
{
    INX_Render3D->renderPass = INX_RenderPass::RENDER_NONE;
    INX_Render3D->renderFlags = 0;

    INX_Render3D->drawCalls.sharedData.Clear();
    INX_Render3D->drawCalls.uniqueData.Clear();
}

static INX_DrawType INX_GetDrawType(const NX_Material& material)
{
    if (material.depth.prePass) return DRAW_PREPASS;
    if (material.blend != NX_BLEND_OPAQUE) {
        return DRAW_TRANSPARENT;
    }
    return DRAW_OPAQUE;
}

static int INX_ComputeBoneMatrices(const NX_Model& model)
{
    const NX_Skeleton& skeleton = *model.skeleton;

    const NX_Mat4* currentPose = (model.player != nullptr)
        ? model.player->currentPose : skeleton.bindPose;

    int boneMatrixOffset = -1;
    NX_Mat4* bones = INX_Render3D->drawCalls.boneBuffer.StageMap(skeleton.boneCount, &boneMatrixOffset);
    NX_Mat4MulBatch(bones, skeleton.boneOffsets, currentPose, skeleton.boneCount);

    return boneMatrixOffset;
}

static void INX_PushDrawCall(
    const INX_VariantMesh& mesh, const NX_InstanceBuffer* instances, int instanceCount,
    const NX_Material& material, const NX_Transform& transform)
{
    INX_DrawCallState& state = INX_Render3D->drawCalls;

    int sharedIndex = state.sharedData.GetSize();
    int uniqueIndex = state.uniqueData.GetSize();

    state.sharedData.EmplaceBack(INX_DrawShared {
        .transform = transform,
        .sphere = INX_BoundingSphere3D(mesh.GetAABB(), transform),
        .instances = instances,
        .instanceCount = instanceCount,
        .boneMatrixOffset = -1,
        .uniqueDataIndex = uniqueIndex,
        .uniqueDataCount = 1
    });

    INX_DrawUnique uniqueData{
        .mesh = mesh,
        .material = material,
        .obb = INX_OrientedBoundingBox3D(mesh.GetAABB(), transform),
        .textures = {},
        .dynamicRangeIndex = -1,
        .sharedDataIndex = sharedIndex,
        .uniqueDataIndex = uniqueIndex,
        .type = INX_GetDrawType(material)
    };

    if (material.shader != nullptr) {
        uniqueData.textures = material.shader->GetTextures();
        uniqueData.dynamicRangeIndex = material.shader->GetDynamicRangeIndex();
    }

    state.uniqueData.PushBack(uniqueData);
}

static void INX_PushDrawCall(
    const NX_Model& model, const NX_InstanceBuffer* instances,
    int instanceCount, const NX_Transform& transform)
{
    INX_DrawCallState& state = INX_Render3D->drawCalls;

    /* --- If the model is rigged we process the bone matrices --- */

    int boneMatrixOffset = -1;
    if (model.skeleton != nullptr) {
        boneMatrixOffset = INX_ComputeBoneMatrices(model);
    }

    /* --- Push draw calls data --- */

    int sharedIndex = state.sharedData.GetSize();
    int uniqueIndex = state.uniqueData.GetSize();

    state.sharedData.EmplaceBack(INX_DrawShared {
        .transform = transform,
        .sphere = INX_BoundingSphere3D(model.aabb, transform),
        .instances = instances,
        .instanceCount = instanceCount,
        .boneMatrixOffset = boneMatrixOffset,
        .uniqueDataIndex = uniqueIndex,
        .uniqueDataCount = model.meshCount
    });

    for (int i = 0; i < model.meshCount; ++i)
    {
        INX_DrawUnique uniqueData{
            .mesh = model.meshes[i],
            .material = model.materials[model.meshMaterials[i]],
            .obb = INX_OrientedBoundingBox3D(model.aabb, transform),
            .textures = {},
            .dynamicRangeIndex = -1,
            .sharedDataIndex = sharedIndex,
            .uniqueDataIndex = static_cast<int>(state.uniqueData.GetSize()),
            .type = INX_GetDrawType(model.materials[model.meshMaterials[i]])
        };

        if (uniqueData.material.shader != nullptr) {
            uniqueData.textures = uniqueData.material.shader->GetTextures();
            uniqueData.dynamicRangeIndex = uniqueData.material.shader->GetDynamicRangeIndex();
        }

        state.uniqueData.PushBack(uniqueData);
    }
}

static void INX_UploadDrawCalls()
{
    INX_DrawCallState& state = INX_Render3D->drawCalls;

    state.boneBuffer.Upload();

    const size_t sharedCount = state.sharedData.GetSize();
    const size_t uniqueCount = state.uniqueData.GetSize();
    const size_t sharedBytes = sharedCount * sizeof(INX_GPUDrawShared);
    const size_t uniqueBytes = uniqueCount * sizeof(INX_GPUDrawUnique);

    state.sharedBuffer.Reserve(sharedBytes, false);
    state.uniqueBuffer.Reserve(uniqueBytes, false);

    INX_GPUDrawShared* sharedBuffer = state.sharedBuffer.MapRange<INX_GPUDrawShared>(
        0, sharedBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    INX_GPUDrawUnique* uniqueBuffer = state.uniqueBuffer.MapRange<INX_GPUDrawUnique>(
        0, uniqueBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    for (size_t i = 0; i < sharedCount; i++)
    {
        const INX_DrawShared& shared = state.sharedData[i];

        NX_Mat3 matNormal = NX_TransformToNormalMat3(&shared.transform);

        INX_GPUDrawShared& gpuShared = sharedBuffer[i];
        gpuShared.matModel = NX_TransformToMat4(&shared.transform);
        gpuShared.matNormal = NX_Mat3ToMat4(&matNormal);
        gpuShared.boneOffset = shared.boneMatrixOffset;
        gpuShared.instancing = (shared.instanceCount > 0);
        gpuShared.skinning = (shared.boneMatrixOffset >= 0);

        const int uniqueStart = shared.uniqueDataIndex;
        const int uniqueEnd = uniqueStart + shared.uniqueDataCount;

        for (int j = uniqueStart; j < uniqueEnd; j++)
        {
            const INX_DrawUnique& unique = state.uniqueData[j];
            const NX_Material& material = unique.material;

            INX_GPUDrawUnique& gpuUnique = uniqueBuffer[j];
            gpuUnique.albedoColor = NX_ColorToVec4(material.albedo.color);
            gpuUnique.emissionColor = NX_ColorToVec3(material.emission.color);
            gpuUnique.emissionEnergy = material.emission.energy;
            gpuUnique.aoLightAffect = material.orm.aoLightAffect;
            gpuUnique.occlusion = material.orm.occlusion;
            gpuUnique.roughness = material.orm.roughness;
            gpuUnique.metalness = material.orm.metalness;
            gpuUnique.normalScale = material.normal.scale;
            gpuUnique.alphaCutOff = material.alphaCutOff;
            gpuUnique.depthOffset = material.depth.offset;
            gpuUnique.depthScale = material.depth.scale;
            gpuUnique.texOffset = material.texOffset;
            gpuUnique.texScale = material.texScale;
            gpuUnique.billboard = material.billboard;
            gpuUnique.layerMask = unique.mesh.GetLayerMask();
        }
    }

    state.sharedBuffer.Unmap();
    state.uniqueBuffer.Unmap();
}

template <bool FrustumCulling>
static void INX_CullDrawCalls(const INX_Frustum& frustum, NX_Layer frustumCullMask)
{
    INX_DrawCallState& state = INX_Render3D->drawCalls;
    state.uniqueVisible.Clear();

    for (const INX_DrawShared& shared : state.sharedData)
    {
        /* --- Classification by bounding sphere if necessary --- */

        bool fullyInside = false;
        if constexpr (FrustumCulling) {
            if (shared.instanceCount == 0) [[likely]] {
                const INX_Frustum::Containment containment = frustum.ClassifySphere(shared.sphere);
                if (containment == INX_Frustum::Outside) continue;
                fullyInside = (containment == INX_Frustum::Inside);
            }
        }

        /* --- Fine filtering using layer mask and OBB vs Frustum test if necessary --- */

        const int end = shared.uniqueDataIndex + shared.uniqueDataCount;

        for (int i = shared.uniqueDataIndex; i < end; ++i)   {
            const INX_DrawUnique& u = state.uniqueData[i];
            if ((frustumCullMask & u.mesh.GetLayerMask()) != 0) [[likely]] {
                if constexpr (FrustumCulling) {
                    if (fullyInside || frustum.ContainsObb(u.obb)) {
                        state.uniqueVisible.Emplace(u.type, i);
                    }
                }
                else {
                    state.uniqueVisible.Emplace(u.type, i);
                }
            }
        }
    }
}

static void INX_SortDrawCalls(const NX_Vec3& viewPosition)
{
    INX_DrawCallState& state = INX_Render3D->drawCalls;
    util::DynamicArray<float>& sortDistances = state.sortDistances;

    const bool needsOpaque = NX_FLAG_CHECK(INX_Render3D->renderFlags, NX_RENDER_SORT_OPAQUE);
    const bool needsPrepass = NX_FLAG_CHECK(INX_Render3D->renderFlags, NX_RENDER_SORT_PREPASS);
    const bool needsTransparent = NX_FLAG_CHECK(INX_Render3D->renderFlags, NX_RENDER_SORT_TRANSPARENT);

    if (needsOpaque || needsPrepass)
    {
        const size_t count = state.uniqueData.GetSize();
        sortDistances.Resize(count);

        for (size_t i = 0; i < count; ++i)
        {
            const INX_DrawUnique& unique = state.uniqueData[i];
            const INX_DrawShared& shared = state.sharedData[unique.sharedDataIndex];

            const NX_BoundingBox3D& box = unique.mesh.GetAABB();
            const NX_Transform& transform = shared.transform;

            // Distance from view position to the AABB's center

            NX_Vec3 local = (box.min + box.max) * 0.5f;
            NX_Vec3 world = local * transform;

            sortDistances[i] = NX_Vec3DistanceSq(viewPosition, world);
        }

        if (needsOpaque) {
            state.uniqueVisible.Sort(DRAW_OPAQUE, [&sortDistances](int a, int b) {
                return sortDistances[a] < sortDistances[b];
            });
        }

        if (needsPrepass) {
            state.uniqueVisible.Sort(DRAW_PREPASS, [&sortDistances](int a, int b) {
                return sortDistances[a] < sortDistances[b];
            });
        }
    }

    if (needsTransparent)
    {
        const size_t count = state.uniqueData.GetSize();
        sortDistances.Resize(count);

        for (size_t i = 0; i < count; ++i)
        {
            const INX_DrawUnique& unique = state.uniqueData[i];
            const INX_DrawShared& shared = state.sharedData[unique.sharedDataIndex];

            const NX_BoundingBox3D& box = unique.mesh.GetAABB();
            const NX_Transform& transform = shared.transform;

            // Distance from view position to the AABB's farthest corner

            const NX_Vec3 corners[8] = {
                NX_VEC3(box.min.x, box.min.y, box.min.z) * transform,
                NX_VEC3(box.max.x, box.min.y, box.min.z) * transform,
                NX_VEC3(box.min.x, box.max.y, box.min.z) * transform,
                NX_VEC3(box.max.x, box.max.y, box.min.z) * transform,
                NX_VEC3(box.min.x, box.min.y, box.max.z) * transform,
                NX_VEC3(box.max.x, box.min.y, box.max.z) * transform,
                NX_VEC3(box.min.x, box.max.y, box.max.z) * transform,
                NX_VEC3(box.max.x, box.max.y, box.max.z) * transform
            };

            float maxDistSq = NX_Vec3DistanceSq(viewPosition, corners[0]);
            for (int i = 1; i < 8; ++i) {
                float distSq = NX_Vec3DistanceSq(viewPosition, corners[i]);
                if (distSq > maxDistSq) maxDistSq = distSq;
            }

            sortDistances[i] = maxDistSq;
        }

        state.uniqueVisible.Sort(DRAW_TRANSPARENT, [&sortDistances](int a, int b) {
            return sortDistances[a] > sortDistances[b];
        });
    }
}

static void INX_Draw3D(const gpu::Pipeline& pipeline, const INX_DrawUnique& unique, const INX_DrawShared& shared)
{
    /* --- Gets data according to the type of mesh to be drawn --- */

    const NX_Material& material = unique.material;
    const INX_VariantMesh& vMesh = unique.mesh;

    NX_PrimitiveType primitiveType = NX_PRIMITIVE_TRIANGLES;
    NX_VertexBuffer3D* buffer = nullptr;

    switch (vMesh.GetTypeIndex()) {
    case 0: [[likely]]
        {
            const NX_Mesh* mesh = vMesh.Get<0>();
            primitiveType = mesh->primitiveType;
            buffer = mesh->buffer;
        }
        break;
    case 1: [[unlikely]]
        {
            const NX_DynamicMesh* mesh = vMesh.Get<1>();
            primitiveType = mesh->primitiveType;
            buffer = mesh->buffer;
        }
        break;
    default:
        NX_UNREACHABLE();
        break;
    }

    /* --- Draws the mesh according to its parameters --- */

    GLenum primitive = INX_GPU_GetPrimitiveType(primitiveType);
    bool isIndexed = (buffer->ebo.IsValid() && buffer->indexCount > 0);
    bool useInstancing = (shared.instances && shared.instanceCount > 0);

    pipeline.BindVertexArray(buffer->vao);
    if (useInstancing) [[unlikely]] {
        buffer->BindInstances(*shared.instances);
    }

    if (isIndexed) [[likely]] {
        useInstancing ? 
            pipeline.DrawElementsInstanced(primitive, GL_UNSIGNED_INT, buffer->indexCount, shared.instanceCount) :
            pipeline.DrawElements(primitive, GL_UNSIGNED_INT, buffer->indexCount);
    }
    else {
        useInstancing ?
            pipeline.DrawInstanced(primitive, buffer->vertexCount, shared.instanceCount) :
            pipeline.Draw(primitive, buffer->vertexCount);
    }
}

static void INX_Draw3D(const gpu::Pipeline& pipeline, const INX_DrawUnique& unique)
{
    INX_Draw3D(pipeline, unique, INX_Render3D->drawCalls.sharedData[unique.sharedDataIndex]);
}

static NX_Vec4 INX_GetBloomPrefilter(float threshold, float softThreshold)
{
    float knee = threshold * softThreshold;

    NX_Vec4 prefilter;
    prefilter.x = threshold;
    prefilter.y = threshold - knee;
    prefilter.z = 2.0f * knee;
    prefilter.w = 0.25f / (knee + 1e-6f);

    return prefilter;
}

static void INX_ProcessFrustum(const NX_Camera& camera, float aspect)
{
    INX_SceneState& scene = INX_Render3D->scene;
    INX_ViewFrustum& frustum = scene.viewFrustum;

    frustum.cullMask = camera.cullMask;
    frustum.position = camera.position;
    frustum.near = camera.nearPlane;
    frustum.far = camera.farPlane;

    frustum.view = NX_GetCameraViewMatrix(&camera);
    frustum.proj = NX_GetCameraProjectionMatrix(&camera, aspect);
    frustum.viewProj = frustum.view * frustum.proj;

    frustum.invView = NX_Mat4Inverse(&frustum.view);
    frustum.invProj = NX_Mat4Inverse(&frustum.proj);
    frustum.invViewProj = NX_Mat4Inverse(&frustum.viewProj);

    frustum.Update(frustum.viewProj);

    scene.frustumUniform.UploadObject(INX_GPUFrustum {
        .viewProj = frustum.viewProj,
        .view = frustum.view,
        .proj = frustum.proj,
        .invViewProj = frustum.invViewProj,
        .invView = frustum.invView,
        .invProj = frustum.invProj,
        .position = frustum.position,
        .near = frustum.near,
        .far = frustum.far
    });
}

static void INX_ProcessEnvironment(const NX_Environment& env)
{
    INX_SceneState& state = INX_Render3D->scene;

    /* --- Store textures --- */

    state.skyCubemap = env.sky.cubemap;
    state.skyProbe = env.sky.probe;

    /* --- Store CPU data */

    state.background = env.background;

    // Pre-multiply background with fog
    if (env.fog.mode != NX_FOG_DISABLED) {
        state.background = NX_ColorLerp(state.background, env.fog.color, env.fog.skyAffect);
    }

    // Calculation of physical bloom level factors
    if (env.bloom.mode != NX_BLOOM_DISABLED)
    {
        state.bloomLevels.Clear();

        int bloomMipCount = state.mipChain.GetNumLevels();
        if (!state.bloomLevels.Reserve(bloomMipCount)) {
            NX_LOG(E, "RENDER: Bloom mip factor buffer reservation failed (requested: %d levels)", bloomMipCount);
        }

        for (uint32_t i = 0; i < bloomMipCount; ++i) {
            float t = float(i) / float(bloomMipCount - 1); // 0 -> 1
            float mapped = t * (int(NX_ARRAY_SIZE(env.bloom.levels)) - 1);
            uint32_t idx0 = uint32_t(mapped);
            uint32_t idx1 = NX_MIN(idx0 + 1, uint32_t(NX_ARRAY_SIZE(env.bloom.levels) - 1));
            float frac = mapped - idx0;
            state.bloomLevels.EmplaceBack(
                env.bloom.levels[idx0] * (1.0f - frac) +
                env.bloom.levels[idx1] * frac
            );
        }
    }

    state.tonemapMode = env.tonemap.mode;
    state.ssaoEnabled = env.ssao.enabled;
    state.bloomMode = env.bloom.mode;

    /* --- Get all GPU data --- */

    INX_GPUEnvironment data{};

    data.ambientColor = NX_VEC3(env.ambient.r, env.ambient.g, env.ambient.b);
    data.skyRotation = NX_VEC4(env.sky.rotation.x, env.sky.rotation.y, env.sky.rotation.z, env.sky.rotation.w);
    data.skyIntensity = env.sky.intensity;
    data.skySpecular = env.sky.specular * env.sky.intensity;
    data.skyDiffuse = env.sky.diffuse * env.sky.intensity;

    data.fogDensity = env.fog.density;
    data.fogStart = env.fog.start;
    data.fogEnd = env.fog.end;
    data.fogSkyAffect = (env.fog.mode) ? env.fog.skyAffect : 0.0f;
    data.fogColor = NX_VEC3(env.fog.color.r, env.fog.color.g, env.fog.color.b);
    data.fogMode = env.fog.mode;

    data.ssaoIntensity = env.ssao.intensity;
    data.ssaoRadius = env.ssao.radius;
    data.ssaoPower = env.ssao.power;
    data.ssaoBias = env.ssao.bias;
    data.ssaoEnabled = env.ssao.enabled;

    data.bloomPrefilter = INX_GetBloomPrefilter(env.bloom.threshold, env.bloom.softThreshold);
    data.bloomFilterRadius = env.bloom.filterRadius;
    data.bloomStrength = env.bloom.strength;
    data.bloomMode = env.bloom.mode;

    data.adjustBrightness = env.adjustment.brightness;
    data.adjustContrast = env.adjustment.contrast;
    data.adjustSaturation = env.adjustment.saturation;

    data.tonemapExposure = env.tonemap.exposure;
    data.tonemapWhite = env.tonemap.white;
    data.tonemapMode = env.tonemap.mode;

    /* --- Upload GPU data --- */

    state.envUniform.Upload(&data);
}

static void INX_CollectActiveLights()
{
    INX_LightingState& state = INX_Render3D->lighting;

    /* --- Clear the previous state --- */

    state.activeLights.Clear();
    state.activeShadows.Clear();

    /* --- Count each active and visible light per type --- */

    NX_Layer viewCullMask = INX_Render3D->scene.viewFrustum.cullMask;

    std::array<size_t, NX_LIGHT_TYPE_COUNT> counts{};
    for (const NX_Light& light : INX_Pool.Get<NX_Light>()) {
        if (light.active && (viewCullMask & light.layerMask) != 0) {
            ++counts[light.type];
        }
    }

    size_t totalLights = std::accumulate(counts.begin(), counts.end(), 0);

    if (!state.activeLights.Resize(totalLights)) {
        NX_LOG(W, "RENDER: Failed to reserve space for %d active lights", totalLights);
    }

    /* --- Prepare offsets for each light type --- */

    std::array<size_t, NX_LIGHT_TYPE_COUNT> offsets{};

    offsets[NX_LIGHT_DIR]  = 0;
    offsets[NX_LIGHT_SPOT] = counts[NX_LIGHT_DIR];
    offsets[NX_LIGHT_OMNI] = counts[NX_LIGHT_DIR] + counts[NX_LIGHT_SPOT];

    /* --- Collect all active and visible lights --- */

    for (NX_Light& light : INX_Pool.Get<NX_Light>())
    {
        if (!light.active || (viewCullMask & light.layerMask) == 0) {
            continue;
        }

        int32_t shadowIndex = -1;
        if (light.shadow.active) {
            shadowIndex = state.activeShadows.GetSize();
            state.activeShadows.Emplace(light.type, &light);
        }

        size_t& offset = offsets[light.type];
        state.activeLights[offset++] = INX_ActiveLight(&light, shadowIndex);
    }
}

static void INX_UploadLightData()
{
    INX_LightingState& state = INX_Render3D->lighting;

    if (state.activeLights.IsEmpty()) {
        return;
    }

    state.storageLights.Reserve(INX_Pool.Get<NX_Light>().GetSize() * sizeof(INX_GPULight), false);
    INX_GPULight* mappedLights = state.storageLights.MapRange<INX_GPULight>(
        0, state.activeLights.GetSize() * sizeof(INX_GPULight),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    for (int i = 0; i < state.activeLights.GetSize(); i++) {
        const INX_ActiveLight& data = state.activeLights[i];
        INX_FillGPULight(data.light, &mappedLights[i], data.shadowIndex);
    }

    state.storageLights.Unmap();
}

static void INX_UploadShadowData()
{
    INX_LightingState& state = INX_Render3D->lighting;

    if (state.activeShadows.IsEmpty()) {
        return;
    }

    state.storageShadow.Reserve(state.activeShadows.GetSize() * sizeof(INX_GPUShadow), false);
    INX_GPUShadow* mappedShadows = state.storageShadow.MapRange<INX_GPUShadow>(
        0, state.activeShadows.GetSize() * sizeof(INX_GPUShadow),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    for (int i = 0; i < state.activeShadows.GetSize(); i++) {
        const NX_Light* light = state.activeShadows[i];
        INX_FillGPUShadow(light, &mappedShadows[i]);
    }

    state.storageShadow.Unmap();
}

static void INX_ComputeClusters()
{
    INX_LightingState& state = INX_Render3D->lighting;
    INX_SceneState& scene = INX_Render3D->scene;

    /* --- Early exit if no active light --- */

    if (state.activeLights.IsEmpty()) {
        return;
    }

    /* --- Adapt the number of clusters in Z according to the view frustum --- */

    // SlicesPerDepthOctave defines how many logarithmically-distributed depth slices are
    // allocated per doubling of distance from the near plane. Higher values increase
    // cluster resolution near the camera, improving light culling precision.

    float near = scene.viewFrustum.near;
    float far = scene.viewFrustum.far;

    state.clusterCount.z = std::clamp(int(std::log2(far / near) * state.SlicesPerDepthOctave), 16, 64);
    int clusterTotal = state.clusterCount.x * state.clusterCount.y * state.clusterCount.z;

    state.storageClusters.Reserve(clusterTotal * 4 * sizeof(uint32_t), false);
    state.storageIndices.Reserve(clusterTotal * state.MaxLightsPerCluster * sizeof(uint32_t), false);
    state.storageClusterAABB.Reserve(clusterTotal * (sizeof(NX_Vec4) + sizeof(NX_Vec3)), false); //< minBounds and maxBounds with padding

    /* --- Calculate the Z-slicing parameters --- */

    state.clusterSliceScale = float(state.clusterCount.z) / std::log2(far / near);
    state.clusterSliceBias = -float(state.clusterCount.z) * std::log2(near) / std::log2(far / near);

    /* --- Obtaining the lights affecting each tile --- */

    gpu::Pipeline pipeline;
    pipeline.UseProgram(INX_Programs.GetLightCulling());

    pipeline.BindUniform(0, scene.frustumUniform);
    pipeline.BindStorage(0, state.storageLights);
    pipeline.BindStorage(1, state.storageClusters);
    pipeline.BindStorage(2, state.storageIndices);
    pipeline.BindStorage(3, state.storageClusterAABB);

    pipeline.SetUniformUint3(0, state.clusterCount);
    pipeline.SetUniformFloat1(1, state.clusterSliceScale);
    pipeline.SetUniformFloat1(2, state.clusterSliceBias);
    pipeline.SetUniformUint1(3, state.activeLights.GetSize());
    pipeline.SetUniformUint1(4, state.MaxLightsPerCluster);

    pipeline.DispatchCompute(
        NX_DIV_CEIL(state.clusterCount.x, 4),
        NX_DIV_CEIL(state.clusterCount.y, 4),
        NX_DIV_CEIL(state.clusterCount.z, 4)
    );
}

static void INX_RenderBackground(const gpu::Pipeline& pipeline)
{
    INX_SceneState& scene = INX_Render3D->scene;

    pipeline.BindFramebuffer(scene.framebufferScene);
    pipeline.SetViewport(scene.framebufferScene);
    pipeline.SetDepthMode(gpu::DepthMode::WriteOnly);

    pipeline.ClearDepth(1.0f);
    pipeline.ClearColor(0, scene.background);
    pipeline.ClearColor(1, NX_COLOR(0.25f, 0.25f, 1.0f, 1.0f));

    if (scene.skyCubemap == nullptr) {
        return;
    }

    scene.framebufferScene.SetDrawBuffers({0});

    pipeline.BindUniform(1, scene.frustumUniform);
    pipeline.BindUniform(2, scene.envUniform);

    pipeline.SetDepthMode(gpu::DepthMode::Disabled);
    pipeline.UseProgram(INX_Programs.GetSkybox());

    pipeline.BindTexture(0, scene.skyCubemap->gpu);
    pipeline.Draw(GL_TRIANGLES, 36);

    scene.framebufferScene.EnableDrawBuffers();
}

static void INX_RenderPrePass(const gpu::Pipeline& pipeline)
{
    const INX_DrawCallState& drawCalls = INX_Render3D->drawCalls;
    const INX_SceneState& scene = INX_Render3D->scene;

    if (drawCalls.uniqueVisible.GetCategory(DRAW_PREPASS).IsEmpty()) {
        return;
    }

    pipeline.SetDepthMode(gpu::DepthMode::TestAndWrite);
    pipeline.SetColorWrite(gpu::ColorWrite::Disabled);

    pipeline.BindStorage(0, drawCalls.sharedBuffer);
    pipeline.BindStorage(1, drawCalls.uniqueBuffer);
    pipeline.BindStorage(2, drawCalls.boneBuffer.GetBuffer());

    pipeline.BindUniform(0, scene.frameUniform);
    pipeline.BindUniform(1, scene.frustumUniform);
    pipeline.BindUniform(2, scene.envUniform);

    for (int uniqueIndex : drawCalls.uniqueVisible.GetCategory(DRAW_PREPASS))
    {
        const INX_DrawUnique& unique = drawCalls.uniqueData[uniqueIndex];
        const NX_Material& mat = unique.material;

        const NX_Shader3D* shader = INX_Assets.Select(mat.shader, INX_Shader3DAsset::DEFAULT);
        pipeline.UseProgram(shader->GetProgram(NX_Shader3D::Variant::SCENE_PREPASS));

        pipeline.SetDepthFunc(INX_GPU_GetDepthFunc(mat.depth.test));
        pipeline.SetCullMode(INX_GPU_GetCullMode(mat.cull));

        shader->BindTextures(pipeline, unique.textures);
        shader->BindUniforms(pipeline, unique.dynamicRangeIndex);

        const NX_Texture* texAlbedo = INX_Assets.Select(
            unique.material.albedo.texture,
            INX_TextureAsset::WHITE
        );

        pipeline.BindTexture(0, texAlbedo->gpu);

        pipeline.SetUniformUint1(0, unique.sharedDataIndex);
        pipeline.SetUniformUint1(1, unique.uniqueDataIndex);

        INX_Draw3D(pipeline, unique);
    }
}

static void INX_RenderScene(const gpu::Pipeline& pipeline)
{
    const INX_DrawCallState& drawCalls = INX_Render3D->drawCalls;
    const INX_SceneState& scene = INX_Render3D->scene;

    const INX_ShadowingState& shadowing = INX_Render3D->shadowing;
    const INX_LightingState& lighting = INX_Render3D->lighting;

    pipeline.SetDepthMode(gpu::DepthMode::TestAndWrite);
    pipeline.SetColorWrite(gpu::ColorWrite::RGBA);

    pipeline.BindStorage(0, drawCalls.sharedBuffer);
    pipeline.BindStorage(1, drawCalls.uniqueBuffer);
    pipeline.BindStorage(2, drawCalls.boneBuffer.GetBuffer());
    pipeline.BindStorage(3, lighting.storageLights);
    pipeline.BindStorage(4, lighting.storageShadow);
    pipeline.BindStorage(5, lighting.storageClusters);
    pipeline.BindStorage(6, lighting.storageIndices);

    pipeline.BindTexture(4, INX_Assets.Get(INX_TextureAsset::BRDF_LUT)->gpu);
    pipeline.BindTexture(7, shadowing.target[NX_LIGHT_DIR]);
    pipeline.BindTexture(8, shadowing.target[NX_LIGHT_SPOT]);
    pipeline.BindTexture(9, shadowing.target[NX_LIGHT_OMNI]);

    pipeline.BindUniform(0, scene.frameUniform);
    pipeline.BindUniform(1, scene.frustumUniform);
    pipeline.BindUniform(2, scene.envUniform);

    if (scene.skyProbe != nullptr) {
        pipeline.BindTexture(5, scene.skyProbe->irradiance);
        pipeline.BindTexture(6, scene.skyProbe->prefilter);
    }

    // Ensures that the SSBOs are ready (especially clusters)
    // Ensures that the generated images are ready (especially reflection probes)
    pipeline.MemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    for (int uniqueIndex : drawCalls.uniqueVisible.GetCategories(DRAW_OPAQUE, DRAW_PREPASS, DRAW_TRANSPARENT))
    {
        const INX_DrawUnique& unique = drawCalls.uniqueData[uniqueIndex];
        const NX_Material& mat = unique.material;

        const NX_Shader3D* shader = INX_Assets.Select(mat.shader, INX_Shader3DAsset::DEFAULT);
        pipeline.UseProgram(shader->GetProgramFromShadingMode(unique.material.shading));

        shader->BindTextures(pipeline, unique.textures);
        shader->BindUniforms(pipeline, unique.dynamicRangeIndex);

        pipeline.SetDepthFunc(mat.depth.prePass ? gpu::DepthFunc::Equal : INX_GPU_GetDepthFunc(mat.depth.test));
        pipeline.SetBlendMode(INX_GPU_GetBlendMode(mat.blend));
        pipeline.SetCullMode(INX_GPU_GetCullMode(mat.cull));

        pipeline.BindTexture(0, INX_Assets.Select(mat.albedo.texture, INX_TextureAsset::WHITE)->gpu);
        pipeline.BindTexture(1, INX_Assets.Select(mat.emission.texture, INX_TextureAsset::WHITE)->gpu);
        pipeline.BindTexture(2, INX_Assets.Select(mat.orm.texture, INX_TextureAsset::WHITE)->gpu);
        pipeline.BindTexture(3, INX_Assets.Select(mat.normal.texture, INX_TextureAsset::NORMAL)->gpu);

        pipeline.SetUniformUint1(0, unique.sharedDataIndex);
        pipeline.SetUniformUint1(1, unique.uniqueDataIndex);

        INX_Draw3D(pipeline, unique);
    }
}

static const gpu::Texture& INX_PostSSAO(const gpu::Texture& source)
{
    INX_SceneState& scene = INX_Render3D->scene;

    // Right now SSAO is done in a simple way by directly darkening
    // the rendered scene, instead of being physically correct.
    // The proper way would be to run a depth pre-pass to get depth
    // and normals of opaque objects, compute SSAO, and apply it to
    // ambient light during the forward pass. But that makes things
    // more complicated for material shaders, which
    // aren’t in yet, and could hurt performance on mobile.
    // So for now we stick with this simpler version until it’s needed.

    gpu::Pipeline pipeline;

    /* --- Bind common stuff --- */

    pipeline.BindUniform(0, scene.frustumUniform);
    pipeline.BindUniform(1, scene.envUniform);

    /* --- Generate ambient occlusion --- */

    pipeline.BindFramebuffer(scene.swapAuxiliary.GetTarget());
    {
        pipeline.SetViewport(scene.swapAuxiliary.GetTarget());
        pipeline.UseProgram(INX_Programs.GetSsaoPass());

        pipeline.BindTexture(0, scene.targetSceneDepth);
        pipeline.BindTexture(1, scene.targetSceneNormal);
        pipeline.BindTexture(2, INX_Assets.Get(INX_TextureAsset::SSAO_KERNEL)->gpu);
        pipeline.BindTexture(3, INX_Assets.Get(INX_TextureAsset::SSAO_NOISE)->gpu);

        pipeline.Draw(GL_TRIANGLES, 3);
    }
    scene.swapAuxiliary.Swap();

    /* --- Blur ambient occlusion --- */

    pipeline.UseProgram(INX_Programs.GetSsaoBilateralBlur());

    pipeline.BindTexture(1, scene.targetSceneDepth);

    pipeline.BindFramebuffer(scene.swapAuxiliary.GetTarget());
    {
        pipeline.BindTexture(0, scene.swapAuxiliary.GetSource());
        pipeline.SetUniformFloat2(0, NX_VEC2(1.0f / scene.swapAuxiliary.GetSource().GetWidth(), 0.0f));
        pipeline.Draw(GL_TRIANGLES, 3);
    }
    scene.swapAuxiliary.Swap();

    pipeline.BindFramebuffer(scene.swapAuxiliary.GetTarget());
    {
        pipeline.BindTexture(0, scene.swapAuxiliary.GetSource());
        pipeline.SetUniformFloat2(0, NX_VEC2(0.0f, 1.0f / scene.swapAuxiliary.GetSource().GetHeight()));
        pipeline.Draw(GL_TRIANGLES, 3);
    }
    scene.swapAuxiliary.Swap();

    /* --- Apply SSAO --- */

    pipeline.BindFramebuffer(scene.swapPostProcess.GetTarget());
    {
        pipeline.SetViewport(scene.swapPostProcess.GetTarget());
        pipeline.UseProgram(INX_Programs.GetSsaoPost());

        pipeline.BindTexture(0, source);
        pipeline.BindTexture(1, scene.swapAuxiliary.GetSource());

        pipeline.Draw(GL_TRIANGLES, 3);
    }
    scene.swapPostProcess.Swap();

    return scene.swapPostProcess.GetSource();
}

static const gpu::Texture& INX_PostBloom(const gpu::Texture& source)
{
    INX_SceneState& scene = INX_Render3D->scene;
    gpu::Pipeline pipeline;

    /* --- Bind common stuff --- */

    pipeline.BindUniform(0, scene.envUniform);

    /* --- Downsampling of the source --- */

    pipeline.UseProgram(INX_Programs.GetDownsampling());

    scene.mipChain.Downsample(pipeline, 0, [&](int targetLevel, int sourceLevel) {
        const gpu::Texture& texSource = (targetLevel == 0) ? source : scene.mipChain.GetTexture();
        pipeline.SetUniformFloat2(0, NX_IVec2Rcp(texSource.GetDimensions()));
        pipeline.SetUniformInt1(1, targetLevel);
        pipeline.BindTexture(0, texSource);
        pipeline.Draw(GL_TRIANGLES, 3);
    });

    /* --- Apply bloom level factors --- */

    pipeline.UseProgram(INX_Programs.GetScreenQuad());
    pipeline.SetBlendMode(gpu::BlendMode::Multiply);

    scene.mipChain.Iterate(pipeline, [&](int targetLevel) {
        pipeline.SetUniformFloat4(0, NX_VEC4_1(scene.bloomLevels[targetLevel]));
        pipeline.Draw(GL_TRIANGLES, 3);
    });

    /* --- Upsampling of the source --- */

    pipeline.UseProgram(INX_Programs.GetUpsampling());
    pipeline.SetBlendMode(gpu::BlendMode::Additive);

    scene.mipChain.Upsample(pipeline, [&](int targetLevel, int sourceLevel) {
        pipeline.Draw(GL_TRIANGLES, 3);
    });

    pipeline.SetBlendMode(gpu::BlendMode::Disabled);

    /* --- Applying bloom to the scene --- */

    pipeline.BindFramebuffer(scene.swapPostProcess.GetTarget());
    pipeline.SetViewport(scene.swapPostProcess.GetTarget());

    pipeline.UseProgram(INX_Programs.GetBloomPost(scene.bloomMode));

    pipeline.BindTexture(0, source);
    pipeline.BindTexture(1, scene.mipChain.GetTexture());

    pipeline.Draw(GL_TRIANGLES, 3);

    scene.swapPostProcess.Swap();

    return scene.swapPostProcess.GetSource();
}

static void INX_PostFinal(const gpu::Texture& source)
{
    INX_SceneState& scene = INX_Render3D->scene;
    gpu::Pipeline pipeline;

    if (scene.target != nullptr) {
        pipeline.BindFramebuffer(scene.target->gpu);
    }
    pipeline.SetViewport(scene.targetResolution);

    pipeline.UseProgram(INX_Programs.GetOutput(scene.tonemapMode));
    pipeline.BindUniform(0, scene.envUniform);
    pipeline.BindTexture(0, source);

    pipeline.Draw(GL_TRIANGLES, 3);
}

// ============================================================================
// PUBLIC API
// ============================================================================

void NX_Begin3D(const NX_Camera* camera, const NX_Environment* env, NX_RenderFlags flags)
{
    NX_BeginEx3D(camera, env, nullptr, flags);
}

void NX_BeginEx3D(const NX_Camera* camera, const NX_Environment* env, const NX_RenderTexture* target, NX_RenderFlags flags)
{
    if (!INX_BeginRenderPass(INX_RenderPass::RENDER_SCENE, flags)) {
        return;
    }

    INX_SceneState& scene = INX_Render3D->scene;

    scene.target = target;
    scene.targetResolution = (target) ? NX_GetRenderTextureSize(target) : NX_GetWindowSize();
    scene.targetAspect = static_cast<float>(scene.targetResolution.x) / scene.targetResolution.y;

    INX_ProcessFrustum(camera ? *camera : NX_GetDefaultCamera(), scene.targetAspect);
    INX_ProcessEnvironment(env ? *env : NX_GetDefaultEnvironment());
}

void NX_End3D()
{
    if (!INX_CheckRenderPass(INX_RenderPass::RENDER_SCENE)) {
        return;
    }

    INX_SceneState& scene = INX_Render3D->scene;

    /* --- Upload draw calls data --- */

    INX_UploadDrawCalls();

    /* --- Process lights --- */

    INX_CollectActiveLights();
    INX_UploadLightData();
    INX_UploadShadowData();
    INX_ComputeClusters();

    /* --- Upload frame info data --- */

    scene.frameUniform.UploadObject(INX_GPUSceneFrame {
        .screenSize = scene.framebufferScene.GetDimensions(),
        .clusterCount = INX_Render3D->lighting.clusterCount,
        .maxLightsPerCluster = INX_Render3D->lighting.MaxLightsPerCluster,
        .clusterSliceScale = INX_Render3D->lighting.clusterSliceScale,
        .clusterSliceBias = INX_Render3D->lighting.clusterSliceBias,
        .elapsedTime = static_cast<float>(NX_GetElapsedTime()),
        .hasActiveLights = (INX_Render3D->lighting.activeLights.GetSize() > 0),
        .hasProbe = (INX_Render3D->scene.skyProbe != nullptr)
    });

    /* --- View layer/furstum culling and sorting --- */

    if (INX_Render3D->renderFlags & NX_RENDER_FRUSTUM_CULLING) {
        INX_CullDrawCalls<true>(scene.viewFrustum, scene.viewFrustum.cullMask);
    }
    else {
        INX_CullDrawCalls<false>(scene.viewFrustum, scene.viewFrustum.cullMask);
    }

    INX_SortDrawCalls(scene.viewFrustum.position);

    /* --- Render scene --- */

    gpu::Pipeline([](const gpu::Pipeline& pipeline) { // NOLINT
        INX_RenderBackground(pipeline);
        INX_RenderPrePass(pipeline);
        INX_RenderScene(pipeline);
    });

    scene.framebufferScene.Resolve();

    /* --- Post process --- */

    const gpu::Texture* source = &scene.targetSceneColor;

    if (scene.ssaoEnabled) {
        source = &INX_PostSSAO(*source);
    }

    if (scene.bloomMode != NX_BLOOM_DISABLED) {
        source = &INX_PostBloom(*source);
    }

    INX_PostFinal(*source);

    /* --- Clear dynamic uniform buffers --- */

    // REVIEW: We can collect the used programs rather than iterating over all programs
    INX_Pool.ForEach<NX_Shader3D>([](NX_Shader3D& shader) {
        shader.ClearDynamicBuffer();
    });

    /* --- Reset state --- */

    INX_EndRenderPass();
}

void NX_BeginShadow3D(NX_Light* light, const NX_Camera* camera, NX_RenderFlags flags)
{
    if (!INX_BeginRenderPass(INX_RenderPass::RENDER_SHADOW, flags)) {
        return;
    }

    if (light->shadow.state.mapIndex < 0) {
        const char* typeName = INX_GetLightTypeName(light->type);
        NX_LOG(W, "RENDER: Light has no valid shadow map assigned (type=%s)", typeName);
        INX_EndRenderPass();
        return;
    }

    INX_Render3D->shadowing.casterTarget = light;
    INX_Render3D->shadowing.mainCamera = camera ? *camera : NX_GetDefaultCamera();
}

void NX_EndShadow3D()
{
    if (!INX_CheckRenderPass(INX_RenderPass::RENDER_SHADOW)) {
        return;
    }

    /* --- Retrieving useful references --- */

    INX_ShadowingState& shadowing = INX_Render3D->shadowing;
    INX_DrawCallState& drawCalls = INX_Render3D->drawCalls;
    INX_LightingState& lighting = INX_Render3D->lighting;
    NX_Light* light = shadowing.casterTarget;

    /* --- Setup common pipeline state --- */

    gpu::Pipeline pipeline;
    pipeline.SetDepthMode(gpu::DepthMode::TestAndWrite);

    pipeline.BindStorage(0, drawCalls.sharedBuffer);
    pipeline.BindStorage(1, drawCalls.uniqueBuffer);
    pipeline.BindStorage(2, drawCalls.boneBuffer.GetBuffer());

    pipeline.BindUniform(0, shadowing.frameUniform);

    /* --- Render shadow maps --- */

    pipeline.BindFramebuffer(shadowing.framebuffer[light->type]);
    pipeline.SetViewport(shadowing.framebuffer[light->type]);

    const int faceCount = (light->type == NX_LIGHT_OMNI) ? 6 : 1;

    for (int face = 0; face < faceCount; ++face)
    {
        /* --- Update and get caster's data --- */

        NX_Mat4 viewProj;
        NX_Vec3 position;
        float range;

        switch (light->type) {
        case NX_LIGHT_DIR:
            viewProj = INX_GetDirectionalLightViewProj(light, shadowing.mainCamera);
            range = std::get<INX_DirectionalLight>(light->data).range;
            position = NX_VEC3_ZERO;
            break;
        case NX_LIGHT_SPOT:
            viewProj = INX_GetSpotLightViewProj(light);
            position = std::get<INX_SpotLight>(light->data).position;
            range = std::get<INX_SpotLight>(light->data).range;
            break;
        case NX_LIGHT_OMNI:
            viewProj = INX_GetOmniLightViewProj(light, face);
            position = std::get<INX_OmniLight>(light->data).position;
            range = std::get<INX_OmniLight>(light->data).range;
            break;
        case NX_LIGHT_TYPE_COUNT:
            NX_UNREACHABLE();
            break;
        }

        /* --- Upload frame uniform --- */

        NX_Mat4 camView = NX_GetCameraViewMatrix(&shadowing.mainCamera);

        shadowing.frameUniform.UploadObject(INX_GPUShadowFrame {
            .lightViewProj = viewProj,
            .cameraInvView = NX_Mat4Inverse(&camView),
            .lightPosition = position,
            .lightRange = range,
            .lightType = light->type,
            .elapsedTime = static_cast<float>(NX_GetElapsedTime())
        });

        /* --- Setup shadow map face --- */

        shadowing.framebuffer[light->type].SetColorAttachmentTarget(
            0, light->shadow.state.mapIndex, face
        );

        pipeline.Clear(
            shadowing.framebuffer[light->type],
            NX_COLOR_1(NX_GetLightRange(light))
        );

        /* --- Cull and render shadow casters --- */

        if (INX_Render3D->renderFlags & NX_RENDER_FRUSTUM_CULLING) {
            INX_CullDrawCalls<true>(INX_Frustum(viewProj), light->shadow.cullMask);
        }
        else {
            INX_CullDrawCalls<false>(INX_Frustum(viewProj), light->shadow.cullMask);
        }

        for (int uniqueIndex : drawCalls.uniqueVisible.GetCategories(DRAW_OPAQUE, DRAW_PREPASS, DRAW_TRANSPARENT))
        {
            const INX_DrawUnique& unique = drawCalls.uniqueData[uniqueIndex];
            if (unique.mesh.GetShadowCastMode() == NX_SHADOW_CAST_DISABLED) continue;

            const NX_Shader3D* shader = INX_Assets.Select(unique.material.shader, INX_Shader3DAsset::DEFAULT);
            pipeline.UseProgram(shader->GetProgram(NX_Shader3D::Variant::SCENE_SHADOW));
            pipeline.SetCullMode(INX_GPU_GetCullMode(unique.mesh.GetShadowFaceMode(), unique.material.cull));

            shader->BindTextures(pipeline, unique.textures);
            shader->BindUniforms(pipeline, unique.dynamicRangeIndex);

            const NX_Texture* texAlbedo = INX_Assets.Select(
                unique.material.albedo.texture,
                INX_TextureAsset::WHITE
            );

            pipeline.BindTexture(0, texAlbedo->gpu);
            pipeline.SetUniformUint1(0, unique.sharedDataIndex);
            pipeline.SetUniformUint1(1, unique.uniqueDataIndex);

            INX_Draw3D(pipeline, unique);
        }
    }

    /* --- Reset state --- */

    INX_EndRenderPass();
}

void NX_DrawMesh3D(const NX_Mesh* mesh, const NX_Material* material, const NX_Transform* transform)
{
    INX_PushDrawCall(
        mesh, nullptr, 0,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawMeshInstanced3D(const NX_Mesh* mesh, const NX_InstanceBuffer* instances, int instanceCount,
                            const NX_Material* material, const NX_Transform* transform)
{
    INX_PushDrawCall(
        mesh, instances, instanceCount,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawDynamicMesh3D(const NX_DynamicMesh* dynMesh, const NX_Material* material, const NX_Transform* transform)
{
    INX_PushDrawCall(
        dynMesh, nullptr, 0,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawDynamicMeshInstanced3D(const NX_DynamicMesh* dynMesh, const NX_InstanceBuffer* instances, int instanceCount,
                                   const NX_Material* material, const NX_Transform* transform)
{
    INX_PushDrawCall(
        dynMesh, instances, instanceCount,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawModel3D(const NX_Model* model, const NX_Transform* transform)
{
    INX_PushDrawCall(
        *model, nullptr, 0,
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawModelInstanced3D(const NX_Model* model, const NX_InstanceBuffer* instances, int instanceCount, const NX_Transform* transform)
{
    INX_PushDrawCall(
        *model, instances, instanceCount,
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}
