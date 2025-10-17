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
#include "../../Detail/GPU/Pipeline.hpp"
#include "../Core/Helper.hpp"

#include "../NX_MaterialShader.hpp"
#include "../NX_VertexBuffer.hpp"
#include "../NX_DynamicMesh.hpp"
#include "./Environment.hpp"
#include "./ViewFrustum.hpp"
#include "./VariantMesh.hpp"

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

/** Bounding sphere (used as 'pre-test' during frustum culling) */
struct BoundingSphere {
    NX_Vec3 center;
    float radius;
    BoundingSphere(const NX_BoundingBox& aabb, const NX_Transform& transform) {
        NX_Vec3 localCenter = (aabb.min + aabb.max) * 0.5f;
        NX_Vec3 rotatedCenter = localCenter * transform.rotation;
        center = transform.translation + rotatedCenter;
        NX_Vec3 halfSize = (aabb.max - aabb.min) * 0.5f;
        radius = NX_Vec3Length(halfSize * transform.scale);
    }
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
    /** Additionnal data */
    NX_MaterialShader::TextureArray textures;   //< Array containing the textures linked to the material shader at the time of draw (if any)
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

    /** Culling and sorting */
    void culling(const Frustum& frustum, NX_Layer frustumCullMask);
    void sorting(const ViewFrustum& frustum, const Environment& environment);

    /** Upload methods */
    void upload();

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
        alignas(4) NX_Vec2 texOffset;
        alignas(4) NX_Vec2 texScale;
        alignas(4) int32_t billboard;
        alignas(4) uint32_t layerMask;
    };

private:
    /** Draw call data stored in RAM */
    util::DynamicArray<SharedData> mSharedData;
    util::DynamicArray<UniqueData> mUniqueData;

    /** Sorted draw call array */
    util::BucketArray<int, DrawType, DRAW_TYPE_COUNT> mUniqueVisible;

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
        NX_INTERNAL_LOG(E, "RENDER: Shared draw call data array pre-allocation failed (requested: %i entries)", initialCapacity);
    }

    if (!mUniqueData.reserve(initialCapacity)) {
        NX_INTERNAL_LOG(E, "RENDER: Unique draw call data array pre-allocation failed (requested: %i entries)", initialCapacity);
    }

    if (!mUniqueVisible.reserve(initialCapacity)) {
        NX_INTERNAL_LOG(E, "RENDER: Visible unique draw call list pre-allocation failed (requested: %i entries)", initialCapacity);
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

    if (model.boneCount > 0)
    {
        const NX_Mat4* boneMatrices = model.boneBindPose;

        if (model.animMode == NX_ANIM_INTERNAL && model.anim != nullptr) {
            if (model.boneCount != model.anim->boneCount) {
                NX_INTERNAL_LOG(W, "RENDER: Model and animation bone counts differ");
            }
            int frame = static_cast<int>(model.animFrame + 0.5) % model.anim->frameCount;
            boneMatrices = model.anim->frameGlobalPoses[frame];
        }
        else if (model.animMode == NX_ANIM_CUSTOM && model.boneOverride != nullptr) {
            boneMatrices = model.boneOverride;
        }

        NX_Mat4* bones = mBoneBuffer.stageMap(model.boneCount, &boneMatrixOffset);
        NX_Mat4MulBatch(bones, model.boneOffsets, boneMatrices, model.boneCount);
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

inline void DrawCallManager::culling(const Frustum& frustum, NX_Layer frustumCullMask)
{
    mUniqueVisible.clear();

    for (SharedData& shared : mSharedData)
    {
        if (shared.instanceCount == 0) {
            if (!frustum.containsSphere(shared.sphere.center, shared.sphere.radius)) {
                continue;
            }
        }

        int uniqueStart = shared.uniqueDataIndex;
        int uniqueEnd = uniqueStart + shared.uniqueDataCount;

        for (int i = uniqueStart; i < uniqueEnd; i++)
        {
            UniqueData unique = mUniqueData[i];

            if ((frustumCullMask & unique.mesh.layerMask()) == 0) {
                continue;
            }

            if (shared.instanceCount == 0) {
                if (!frustum.containsObb(unique.mesh.aabb(), shared.transform)) {
                    continue;
                }
            }

            mUniqueVisible.emplace(unique.type, i);
        }
    }
}

inline void DrawCallManager::sorting(const ViewFrustum& frustum, const Environment& environment)
{
    if (environment.hasFlags(NX_ENV_SORT_OPAQUE)) {
        mUniqueVisible.sort(DRAW_OPAQUE,
            [this, &frustum](int a, int b) {
                const DrawUnique& aUnique = mUniqueData[a];
                const DrawUnique& bUnique = mUniqueData[b];
                const DrawShared& aShared = mSharedData[aUnique.sharedDataIndex];
                const DrawShared& bShared = mSharedData[bUnique.sharedDataIndex];
                float maxDistA = frustum.getDistanceSquaredToCenterPoint(aUnique.mesh.aabb(), aShared.transform);
                float maxDistB = frustum.getDistanceSquaredToCenterPoint(bUnique.mesh.aabb(), bShared.transform);
                return maxDistA < maxDistB;
            }
        );
    }

    if (environment.hasFlags(NX_ENV_SORT_PREPASS)) {
        mUniqueVisible.sort(DRAW_PREPASS,
            [this, &frustum](int a, int b) {
                const DrawUnique& aUnique = mUniqueData[a];
                const DrawUnique& bUnique = mUniqueData[b];
                const DrawShared& aShared = mSharedData[aUnique.sharedDataIndex];
                const DrawShared& bShared = mSharedData[bUnique.sharedDataIndex];
                float maxDistA = frustum.getDistanceSquaredToCenterPoint(aUnique.mesh.aabb(), aShared.transform);
                float maxDistB = frustum.getDistanceSquaredToCenterPoint(bUnique.mesh.aabb(), bShared.transform);
                return maxDistA < maxDistB;
            }
        );
    }

    if (environment.hasFlags(NX_ENV_SORT_TRANSPARENT)) {
        mUniqueVisible.sort(DRAW_TRANSPARENT,
            [this, &frustum](int a, int b) {
                const DrawUnique& aUnique = mUniqueData[a];
                const DrawUnique& bUnique = mUniqueData[b];
                const DrawShared& aShared = mSharedData[aUnique.sharedDataIndex];
                const DrawShared& bShared = mSharedData[bUnique.sharedDataIndex];
                float maxDistA = frustum.getDistanceSquaredToFarthestPoint(aUnique.mesh.aabb(), aShared.transform);
                float maxDistB = frustum.getDistanceSquaredToFarthestPoint(bUnique.mesh.aabb(), bShared.transform);
                return maxDistA > maxDistB;
            }
        );
    }
}

inline void DrawCallManager::upload()
{
    mBoneBuffer.upload();

    mSharedBuffer.reserve(mSharedData.size() * sizeof(GPUSharedData), false);
    mUniqueBuffer.reserve(mUniqueData.size() * sizeof(GPUUniqueData), false);

    GPUSharedData* sharedBuffer = mSharedBuffer.mapRange<GPUSharedData>(
        0, mSharedData.size() * sizeof(GPUSharedData),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    GPUUniqueData* uniqueBuffer = mUniqueBuffer.mapRange<GPUUniqueData>(
        0, mUniqueData.size() * sizeof(GPUUniqueData),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    for (int i = 0; i < mSharedData.size(); i++)
    {
        const SharedData& shared = mSharedData[i];

        NX_Mat4 matModel = NX_TransformToMat4(&shared.transform);
        NX_Mat3 matNormal = NX_Mat3Normal(&matModel);

        sharedBuffer[i] = GPUSharedData {
            .matModel = matModel,
            .matNormal = NX_Mat3ToMat4(&matNormal),
            .boneOffset = shared.boneMatrixOffset,
            .instancing = (shared.instanceCount > 0),
            .skinning = (shared.boneMatrixOffset >= 0)
        };

        int uniqueStart = shared.uniqueDataIndex;
        int uniqueEnd = uniqueStart + shared.uniqueDataCount;

        for (int j = uniqueStart; j < uniqueEnd; j++)
        {
            const UniqueData& unique = mUniqueData[j];
            const NX_Material& material = unique.material;
            const VariantMesh& mesh = unique.mesh;

            uniqueBuffer[j] = GPUUniqueData {
                .albedoColor = NX_ColorToVec4(material.albedo.color),
                .emissionColor = NX_ColorToVec3(material.emission.color),
                .emissionEnergy = material.emission.energy,
                .aoLightAffect = material.orm.aoLightAffect,
                .occlusion = material.orm.occlusion,
                .roughness = material.orm.roughness,
                .metalness = material.orm.metalness,
                .normalScale = material.normal.scale,
                .alphaCutOff = material.alphaCutOff,
                .texOffset = material.texOffset,
                .texScale = material.texScale,
                .billboard = material.billboard,
                .layerMask = mesh.layerMask()
            };
        }
    }

    mSharedBuffer.unmap();
    mUniqueBuffer.unmap();
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

    GLenum primitive = render::getPrimitiveType(primitiveType);
    bool useInstancing = (shared.instances && shared.instanceCount > 0);
    bool hasEBO = buffer->ebo().isValid();

    pipeline.bindVertexArray(buffer->vao());
    if (useInstancing) {
        buffer->bindInstances(*shared.instances);
    }

    if (hasEBO) {
        useInstancing ? 
            pipeline.drawElementsInstanced(primitive, GL_UNSIGNED_INT, indexCount, shared.instanceCount) :
            pipeline.drawElements(primitive, GL_UNSIGNED_INT, indexCount);
    } else {
        useInstancing ?
            pipeline.drawInstanced(primitive, vertexCount, shared.instanceCount) :
            pipeline.draw(primitive, vertexCount);
    }

    if (useInstancing) {
        buffer->unbindInstances();
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
