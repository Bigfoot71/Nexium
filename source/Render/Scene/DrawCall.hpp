/* DrawCall.hpp -- Represents a draw call for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_DRAW_CALL_HPP
#define NX_SCENE_DRAW_CALL_HPP

#include <NX/NX_Macros.h>
#include <NX/NX_Render.h>
#include <NX/NX_Math.h>

#include "../../Detail/Util/BucketArray.hpp"
#include "../../Detail/GPU/Pipeline.hpp"
#include "../Core/Helper.hpp"

#include "../NX_MaterialShader.hpp"
#include "../NX_VertexBuffer.hpp"
#include "../NX_DynamicMesh.hpp"

#include <variant>

namespace scene {

/* === Declaration === */

class DrawCall {
public:
    enum Category {
        OPAQUE = 0,         ///< Represents all purely opaque objects
        PREPASS = 1,        ///< Represents objects rendered with a depth pre-pass (can be opaque or transparent)
        TRANSPARENT = 2,    ///< Represents all transparent objects
        CATEGORY_COUNT
    };

public:
    /** Constructors */
    template<typename T_Mesh>
    DrawCall(int dataIndex, int materialIndex, T_Mesh& mesh, const NX_Material& material);

    /** Draw call category management */
    static Category category(const NX_Material& material);
    Category category() const;

    /** Object data */
    NX_ShadowCastMode shadowCastMode() const;
    NX_ShadowFaceMode shadowFaceMode() const;
    const NX_BoundingBox& aabb() const;
    NX_Layer layerMask() const;

    /** Material data */
    const NX_Material& material() const;
    int materialIndex() const;

    /** External draw call data */
    const NX_MaterialShader::TextureArray& materialShaderTextures() const;
    int dynamicRangeIndex() const;
    int drawDataIndex() const;

    /** Draw command */
    void draw(const gpu::Pipeline& pipeline, const NX_InstanceBuffer* instances, int instanceCount) const;

private:
    /** Object to draw */
    std::variant<const NX_Mesh*, const NX_DynamicMesh*> mMesh;

    /** Material data */
    NX_Material mMaterial{};    //< REVIEW: We don't need to store all the data here
    int mMaterialIndex{};       //< Index pointing to material data stored in the global SSBO

    /** Additionnal data */
    NX_MaterialShader::TextureArray mTextures;  //< Array containing the textures linked to the material shader at the time of draw (if any)
    int mDynamicRangeIndex;                     //< Index of the material shader's dynamic uniform buffer range (if any)
    int mDrawDataIndex;                         //< Index to shared drawing data (DrawData)
};

/* === Container === */

using BucketDrawCalls = util::BucketArray<DrawCall, DrawCall::Category, DrawCall::CATEGORY_COUNT>;

/* === Public Implementation === */

template<typename T_Mesh>
inline DrawCall::DrawCall(int dataIndex, int materialIndex, T_Mesh& mesh, const NX_Material& material)
    : mMesh(&mesh), mMaterial(material), mMaterialIndex(materialIndex)
    , mDynamicRangeIndex(-1), mDrawDataIndex(dataIndex)
{
    if (material.shader != nullptr) {
        mDynamicRangeIndex = material.shader->dynamicRangeIndex();
        material.shader->getTextures(mTextures);
    }
}

inline DrawCall::Category DrawCall::category(const NX_Material& material)
{
    if (material.depth.prePass) return PREPASS;
    if (material.blend != NX_BLEND_OPAQUE) {
        return TRANSPARENT;
    }
    return OPAQUE;
}

inline DrawCall::Category DrawCall::category() const
{
    return category(mMaterial);
}

inline NX_ShadowCastMode DrawCall::shadowCastMode() const
{
    switch (mMesh.index()) {
    case 0: [[likely]] return std::get<0>(mMesh)->shadowCastMode;
    case 1: [[unlikely]] return std::get<1>(mMesh)->shadowCastMode;
    default: NX_UNREACHABLE(); break;
    }
}

inline NX_ShadowFaceMode DrawCall::shadowFaceMode() const
{
    switch (mMesh.index()) {
    case 0: [[likely]] return std::get<0>(mMesh)->shadowFaceMode;
    case 1: [[unlikely]] return std::get<1>(mMesh)->shadowFaceMode;
    default: NX_UNREACHABLE(); break;
    }
}

inline const NX_BoundingBox& DrawCall::aabb() const
{
    switch (mMesh.index()) {
    case 0: [[likely]] return std::get<0>(mMesh)->aabb;
    case 1: [[unlikely]] return std::get<1>(mMesh)->aabb();
    default: NX_UNREACHABLE(); break;
    }
}

inline NX_Layer DrawCall::layerMask() const
{
    switch (mMesh.index()) {
    case 0: [[likely]] return std::get<0>(mMesh)->layerMask;
    case 1: [[unlikely]] return std::get<1>(mMesh)->layerMask;
    default: NX_UNREACHABLE(); break;
    }
}

inline const NX_Material& DrawCall::material() const
{
    return mMaterial;
}

inline int DrawCall::materialIndex() const
{
    return mMaterialIndex;
}

inline const NX_MaterialShader::TextureArray& DrawCall::materialShaderTextures() const
{
    return mTextures;
}

inline int DrawCall::dynamicRangeIndex() const
{
    return mDynamicRangeIndex;
}

inline int DrawCall::drawDataIndex() const
{
    return mDrawDataIndex;
}

inline void DrawCall::draw(const gpu::Pipeline& pipeline, const NX_InstanceBuffer* instances, int instanceCount) const
{
    NX_PrimitiveType primitiveType = NX_PRIMITIVE_TRIANGLES;
    NX_VertexBuffer* buffer = nullptr;
    size_t vertexCount = 0;
    size_t indexCount = 0;

    switch (mMesh.index()) {
    case 0: [[likely]]
        {
            const NX_Mesh* mesh = std::get<0>(mMesh);
            primitiveType = mesh->primitiveType;
            vertexCount = mesh->vertexCount;
            indexCount = mesh->indexCount;
            buffer = mesh->buffer;
        }
        break;
    case 1: [[unlikely]]
        {
            const NX_DynamicMesh* mesh = std::get<1>(mMesh);
            primitiveType = mesh->primitiveType();
            vertexCount = mesh->vertexCount();
            buffer = &mesh->buffer();
        }
        break;
    default:
        NX_UNREACHABLE();
        break;
    }

    GLenum primitive = render::getPrimitiveType(primitiveType);
    bool useInstancing = (instances && instanceCount > 0);
    bool hasEBO = buffer->ebo().isValid();

    pipeline.bindVertexArray(buffer->vao());
    if (useInstancing) {
        buffer->bindInstances(*instances);
    }

    if (hasEBO) {
        useInstancing ? 
            pipeline.drawElementsInstanced(primitive, GL_UNSIGNED_INT, indexCount, instanceCount) :
            pipeline.drawElements(primitive, GL_UNSIGNED_INT, indexCount);
    } else {
        useInstancing ?
            pipeline.drawInstanced(primitive, vertexCount, instanceCount) :
            pipeline.draw(primitive, vertexCount);
    }

    if (useInstancing) {
        buffer->unbindInstances();
    }
}

} // namespace scene

#endif // NX_SCENE_DRAW_CALL_HPP
