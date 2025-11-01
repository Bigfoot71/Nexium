/* DrawCallManager.cpp -- Draw call management class for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_DRAW_CALL_MANAGER_HPP
#define NX_SCENE_DRAW_CALL_MANAGER_HPP

#include "../../Detail/Util/BucketArray.hpp"
#include "../../Detail/GPU/StagingBuffer.hpp"
#include "../../Detail/GPU/Translation.hpp"
#include "../../Detail/GPU/Pipeline.hpp"

#include "../NX_Shader3D.hpp"
#include "../NX_VertexBuffer.hpp"
#include "../NX_DynamicMesh.hpp"
#include "../Core/Helper.hpp"
#include "./Environment.hpp"
#include "./ViewFrustum.hpp"
#include "./VariantMesh.hpp"
#include "./Culling.hpp"
#include "./Frustum.hpp"

#include <NX/NX_Render.h>
#include <NX/NX_Math.h>

namespace scene {

/* === Related Types === */

enum DrawType : uint8_t {
    DRAW_OPAQUE = 0,        ///< Represents all purely opaque objects
    DRAW_PREPASS = 1,       ///< Represents objects rendered with a depth pre-pass (can be opaque or transparent)
    DRAW_TRANSPARENT = 2,   ///< Represents all transparent objects
    DRAW_TYPE_COUNT
};

/** Shared CPU data per draw call */
struct DrawShared {
    /** Spatial data */
    NX_Transform transform;
    BoundingSphere sphere;
    /** Instances data */
    const NX_InstanceBuffer* instances;
    int instanceCount;
    /** Animations */
    int boneMatrixOffset;                       //< If less than zero, no animation assigned
    /** Unique data */
    int uniqueDataIndex;
    int uniqueDataCount;
};

/** Unique CPU data per draw call */
struct DrawUnique {
    /** Object to draw */
    VariantMesh mesh;
    NX_Material material;
    OrientedBoundingBox obb;
    /** Additionnal data */
    NX_Shader3D::TextureArray textures;   //< Array containing the textures linked to the material shader at the time of draw (if any)
    int dynamicRangeIndex;                      //< Index of the material shader's dynamic uniform buffer range (if any)
    /** Shared/Unique data */
    int sharedDataIndex;                        //< Index to the shared data that this unique draw call data depends on
    int uniqueDataIndex;                        //< Is actually the index of DrawUnique itself, useful when iterating through sorted categories
    /** Object type */
    DrawType type;
};

/* === Declaration === */

class DrawCallManager {
public:
    using SharedData = DrawShared;
    using UniqueData = DrawUnique;

public:
    DrawCallManager(int initialCapacity);

    /** Push/clear draw calls */
    void push(const VariantMesh& mesh, const NX_InstanceBuffer* instances, int instanceCount, const NX_Material& material, const NX_Transform& transform);
    void push(const NX_Model& model, const NX_InstanceBuffer* instances, int instanceCount, const NX_Transform& transform);
    void clear();

    /** Upload methods */
    void upload();

    /** Culling and sorting */
    void culling(const Frustum& frustum, NX_Layer frustumCullMask);
    void sorting(const ViewFrustum& frustum, const Environment& environment);

    /** Draw */
    void draw(const gpu::Pipeline& pipeline, const UniqueData& unique, const SharedData& shared) const;
    void draw(const gpu::Pipeline& pipeline, const UniqueData& unique) const;

    /** Getters */
    const auto& sharedData() const;
    const auto& uniqueData() const;
    const auto& uniqueVisible() const;
    const gpu::Buffer& sharedBuffer() const;
    const gpu::Buffer& uniqueBuffer() const;
    const gpu::Buffer& boneBuffer() const;

private:
    /** Internal operations */
    int computeBoneMatrices(const NX_Model& model);

    /** Helpers */
    static DrawType drawType(const NX_Material& material);

private:
    /** Shared GPU data per draw call */
    struct GPUSharedData {
        alignas(16) NX_Mat4 matModel;
        alignas(16) NX_Mat4 matNormal;
        alignas(4) int32_t boneOffset;
        alignas(4) int32_t instancing;
        alignas(4) int32_t skinning;
    };

    /** Unique GPU data per draw call */
    struct GPUUniqueData {
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

private:
    /** Draw call data stored in RAM */
    util::DynamicArray<SharedData> mSharedData;
    util::DynamicArray<UniqueData> mUniqueData;

    /** Sorted draw call array */
    util::BucketArray<int, DrawType, DRAW_TYPE_COUNT> mUniqueVisible;

    /** Sorting cache */
    util::DynamicArray<float> mSortKeysCenterDist;
    util::DynamicArray<float> mSortKeysFarthestDist;

    /** Draw call data stored in VRAM */
    gpu::Buffer mSharedBuffer, mUniqueBuffer;
    gpu::StagingBuffer<NX_Mat4, 1> mBoneBuffer;
};

/* === Public Implementation === */

inline DrawCallManager::DrawCallManager(int initialCapacity)
    : mSharedBuffer(GL_SHADER_STORAGE_BUFFER, initialCapacity * sizeof(GPUSharedData))
    , mUniqueBuffer(GL_SHADER_STORAGE_BUFFER, initialCapacity * sizeof(GPUUniqueData))
    , mBoneBuffer(GL_SHADER_STORAGE_BUFFER, 1024)
{
    if (!mSharedData.reserve(initialCapacity)) {
        NX_LOG(E, "RENDER: Shared draw call data array pre-allocation failed (requested: %i entries)", initialCapacity);
    }

    if (!mUniqueData.reserve(initialCapacity)) {
        NX_LOG(E, "RENDER: Unique draw call data array pre-allocation failed (requested: %i entries)", initialCapacity);
    }

    if (!mUniqueVisible.reserve(initialCapacity)) {
        NX_LOG(E, "RENDER: Visible unique draw call list pre-allocation failed (requested: %i entries)", initialCapacity);
    }
}

inline void DrawCallManager::push(const VariantMesh& mesh, const NX_InstanceBuffer* instances, int instanceCount, const NX_Material& material, const NX_Transform& transform)
{
    int sharedIndex = mSharedData.size();
    int uniqueIndex = mUniqueData.size();

    mSharedData.emplace_back(SharedData {
        .transform = transform,
        .sphere = BoundingSphere(mesh.aabb(), transform),
        .instances = instances,
        .instanceCount = instanceCount,
        .boneMatrixOffset = -1,
        .uniqueDataIndex = uniqueIndex,
        .uniqueDataCount = 1
    });

    UniqueData uniqueData{
        .mesh = mesh,
        .material = material,
        .obb = OrientedBoundingBox(mesh.aabb(), transform),
        .textures = {},
        .dynamicRangeIndex = -1,
        .sharedDataIndex = sharedIndex,
        .uniqueDataIndex = uniqueIndex,
        .type = drawType(material)
    };

    if (material.shader != nullptr) {
        material.shader->getTextures(uniqueData.textures);
        uniqueData.dynamicRangeIndex = material.shader->dynamicRangeIndex();
    }

    mUniqueData.push_back(uniqueData);
}

inline void DrawCallManager::push(const NX_Model& model, const NX_InstanceBuffer* instances, int instanceCount, const NX_Transform& transform)
{
    /* --- If the model is rigged we process the bone matrices --- */

    int boneMatrixOffset = -1;

    if (model.boneCount > 0) {
        boneMatrixOffset = computeBoneMatrices(model);
    }

    /* --- Push draw calls data --- */

    int sharedIndex = mSharedData.size();
    int uniqueIndex = mUniqueData.size();

    mSharedData.emplace_back(SharedData {
        .transform = transform,
        .sphere = BoundingSphere(model.aabb, transform),
        .instances = instances,
        .instanceCount = instanceCount,
        .boneMatrixOffset = boneMatrixOffset,
        .uniqueDataIndex = uniqueIndex,
        .uniqueDataCount = model.meshCount
    });

    for (int i = 0; i < model.meshCount; ++i)
    {
        UniqueData uniqueData{
            .mesh = model.meshes[i],
            .material = model.materials[model.meshMaterials[i]],
            .obb = OrientedBoundingBox(model.aabb, transform),
            .textures = {},
            .dynamicRangeIndex = -1,
            .sharedDataIndex = sharedIndex,
            .uniqueDataIndex = static_cast<int>(mUniqueData.size()),
            .type = drawType(model.materials[model.meshMaterials[i]])
        };

        if (uniqueData.material.shader != nullptr) {
            uniqueData.material.shader->getTextures(uniqueData.textures);
            uniqueData.dynamicRangeIndex = uniqueData.material.shader->dynamicRangeIndex();
        }

        mUniqueData.push_back(uniqueData);
    }
}

inline void DrawCallManager::clear()
{
    mSharedData.clear();
    mUniqueData.clear();
}

inline void DrawCallManager::upload()
{
    mBoneBuffer.upload();

    const size_t sharedSize = mSharedData.size();
    const size_t uniqueSize = mUniqueData.size();
    const size_t sharedBytes = sharedSize * sizeof(GPUSharedData);
    const size_t uniqueBytes = uniqueSize * sizeof(GPUUniqueData);

    mSharedBuffer.reserve(sharedBytes, false);
    mUniqueBuffer.reserve(uniqueBytes, false);

    GPUSharedData* sharedBuffer = mSharedBuffer.mapRange<GPUSharedData>(
        0, sharedBytes,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    GPUUniqueData* uniqueBuffer = mUniqueBuffer.mapRange<GPUUniqueData>(
        0, uniqueBytes,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    for (size_t i = 0; i < sharedSize; i++)
    {
        const SharedData& shared = mSharedData[i];

        NX_Mat3 matNormal = NX_TransformToNormalMat3(&shared.transform);

        GPUSharedData& gpuShared = sharedBuffer[i];
        gpuShared.matModel = NX_TransformToMat4(&shared.transform);
        gpuShared.matNormal = NX_Mat3ToMat4(&matNormal);
        gpuShared.boneOffset = shared.boneMatrixOffset;
        gpuShared.instancing = (shared.instanceCount > 0);
        gpuShared.skinning = (shared.boneMatrixOffset >= 0);

        const int uniqueStart = shared.uniqueDataIndex;
        const int uniqueEnd = uniqueStart + shared.uniqueDataCount;

        for (int j = uniqueStart; j < uniqueEnd; j++)
        {
            const UniqueData& unique = mUniqueData[j];
            const NX_Material& material = unique.material;

            GPUUniqueData& gpuUnique = uniqueBuffer[j];
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
            gpuUnique.layerMask = unique.mesh.layerMask();
        }
    }

    mSharedBuffer.unmap();
    mUniqueBuffer.unmap();
}

inline void DrawCallManager::culling(const Frustum& frustum, NX_Layer frustumCullMask)
{
    mUniqueVisible.clear();

    for (const SharedData& shared : mSharedData)
    {
        if (shared.instanceCount > 0) [[unlikely]] {
            int end = shared.uniqueDataIndex + shared.uniqueDataCount;
            for (int i = shared.uniqueDataIndex; i < end; ++i) {
                const UniqueData& u = mUniqueData[i];
                if ((frustumCullMask & u.mesh.layerMask()) != 0) {
                    mUniqueVisible.emplace(u.type, i);
                }
            }
            continue;
        }

        const Frustum::Containment containment = frustum.classifySphere(shared.sphere);
        if (containment == Frustum::Outside) {
            continue;
        }

        int end = shared.uniqueDataIndex + shared.uniqueDataCount;
        bool needsObbTest = (containment == Frustum::Intersect);

        for (int i = shared.uniqueDataIndex; i < end; ++i) {
            const UniqueData& u = mUniqueData[i];
            if ((frustumCullMask & u.mesh.layerMask()) != 0) [[likely]] {
                if (!needsObbTest || frustum.containsObb(u.obb)) {
                    mUniqueVisible.emplace(u.type, i);
                }
            }
        }
    }
}

inline void DrawCallManager::sorting(const ViewFrustum& frustum, const Environment& environment)
{
    const bool needsOpaque = environment.hasFlags(NX_ENV_SORT_OPAQUE);
    const bool needsPrepass = environment.hasFlags(NX_ENV_SORT_PREPASS);
    const bool needsTransparent = environment.hasFlags(NX_ENV_SORT_TRANSPARENT);
    
    if (needsOpaque || needsPrepass)
    {
        const size_t count = mUniqueData.size();
        mSortKeysCenterDist.resize(count);
        
        for (size_t i = 0; i < count; ++i) {
            const DrawUnique& unique = mUniqueData[i];
            const DrawShared& shared = mSharedData[unique.sharedDataIndex];
            mSortKeysCenterDist[i] = frustum.getDistanceSqToCenterPoint(
                unique.mesh.aabb(), shared.transform
            );
        }
        
        if (needsOpaque) {
            mUniqueVisible.sort(DRAW_OPAQUE, [this](int a, int b) {
                return mSortKeysCenterDist[a] < mSortKeysCenterDist[b];
            });
        }
        
        if (needsPrepass) {
            mUniqueVisible.sort(DRAW_PREPASS, [this](int a, int b) {
                return mSortKeysCenterDist[a] < mSortKeysCenterDist[b];
            });
        }
    }
    
    if (needsTransparent)
    {
        const size_t count = mUniqueData.size();
        mSortKeysFarthestDist.resize(count);

        for (size_t i = 0; i < count; ++i) {
            const DrawUnique& unique = mUniqueData[i];
            const DrawShared& shared = mSharedData[unique.sharedDataIndex];
            mSortKeysFarthestDist[i] = frustum.getDistanceSqToFarthestPoint(
                unique.mesh.aabb(), shared.transform
            );
        }

        mUniqueVisible.sort(DRAW_TRANSPARENT, [this](int a, int b) {
            return mSortKeysFarthestDist[a] > mSortKeysFarthestDist[b];
        });
    }
}

inline void DrawCallManager::draw(const gpu::Pipeline& pipeline, const UniqueData& unique, const SharedData& shared) const
{
    /* --- Gets data according to the type of mesh to be drawn --- */

    const NX_Material& material = unique.material;
    const VariantMesh& vMesh = unique.mesh;

    NX_PrimitiveType primitiveType = NX_PRIMITIVE_TRIANGLES;
    NX_VertexBuffer* buffer = nullptr;
    size_t vertexCount = 0;
    size_t indexCount = 0;

    switch (vMesh.index()) {
    case 0: [[likely]]
        {
            const NX_Mesh* mesh = vMesh.get<0>();
            primitiveType = mesh->primitiveType;
            vertexCount = mesh->vertexCount;
            indexCount = mesh->indexCount;
            buffer = mesh->buffer;
        }
        break;
    case 1: [[unlikely]]
        {
            const NX_DynamicMesh* mesh = vMesh.get<1>();
            primitiveType = mesh->primitiveType();
            vertexCount = mesh->vertexCount();
            buffer = &mesh->buffer();
        }
        break;
    default:
        NX_UNREACHABLE();
        break;
    }

    /* --- Draws the mesh according to its parameters --- */

    GLenum primitive = gpu::getPrimitiveType(primitiveType);
    bool useInstancing = (shared.instances && shared.instanceCount > 0);
    bool hasEBO = buffer->ebo().isValid();

    pipeline.bindVertexArray(buffer->vao());
    if (useInstancing) [[unlikely]] {
        buffer->bindInstances(*shared.instances);
    }

    if (hasEBO) [[likely]] {
        useInstancing ? 
            pipeline.drawElementsInstanced(primitive, GL_UNSIGNED_INT, indexCount, shared.instanceCount) :
            pipeline.drawElements(primitive, GL_UNSIGNED_INT, indexCount);
    } else {
        useInstancing ?
            pipeline.drawInstanced(primitive, vertexCount, shared.instanceCount) :
            pipeline.draw(primitive, vertexCount);
    }
}

inline void DrawCallManager::draw(const gpu::Pipeline& pipeline, const UniqueData& unique) const
{
    draw(pipeline, unique, mSharedData[unique.sharedDataIndex]);
}

inline const auto& DrawCallManager::sharedData() const
{
    return mSharedData;
}

inline const auto& DrawCallManager::uniqueData() const
{
    return mUniqueData;
}

inline const auto& DrawCallManager::uniqueVisible() const
{
    return mUniqueVisible;
}

inline const gpu::Buffer& DrawCallManager::sharedBuffer() const
{
    return mSharedBuffer;
}

inline const gpu::Buffer& DrawCallManager::uniqueBuffer() const
{
    return mUniqueBuffer;
}

inline const gpu::Buffer& DrawCallManager::boneBuffer() const
{
    return mBoneBuffer.buffer();
}

/* === Private Implementation === */

inline int DrawCallManager::computeBoneMatrices(const NX_Model& model)
{
    int boneMatrixOffset = -1;

    const NX_Mat4* boneMatrices = model.boneBindPose;
    if (model.animMode == NX_ANIM_INTERNAL && model.anim != nullptr) {
        if (model.boneCount != model.anim->boneCount) {
            NX_LOG(W, "RENDER: Model and animation bone counts differ");
        }
        float frame = NX_Wrap(model.animFrame, 0.0f, model.anim->frameCount - 1.0f);
        boneMatrices = model.anim->frameGlobalPoses[static_cast<int>(frame + 0.5f)];
    }
    else if (model.animMode == NX_ANIM_CUSTOM && model.boneOverride != nullptr) {
        boneMatrices = model.boneOverride;
    }

    NX_Mat4* bones = mBoneBuffer.stageMap(model.boneCount, &boneMatrixOffset);
    NX_Mat4MulBatch(bones, model.boneOffsets, boneMatrices, model.boneCount);

    return boneMatrixOffset;
}

inline DrawType DrawCallManager::drawType(const NX_Material& material)
{
    if (material.depth.prePass) return DRAW_PREPASS;
    if (material.blend != NX_BLEND_OPAQUE) {
        return DRAW_TRANSPARENT;
    }
    return DRAW_OPAQUE;
}

} // namespace scene

#endif // NX_SCENE_DRAW_CALL_MANAGER_HPP
