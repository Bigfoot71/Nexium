/* DrawCall.hpp -- Represents a draw call for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_DRAW_CALL_HPP
#define NX_SCENE_DRAW_CALL_HPP

#include <NX/NX_Render.h>
#include <NX/NX_Math.h>

#include "../../Detail/Util/BucketArray.hpp"
#include "../../Detail/GPU/Pipeline.hpp"
#include "../NX_MaterialShader.hpp"
#include "../NX_VertexBuffer.hpp"

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
    DrawCall(int dataIndex, const NX_Mesh& mesh, const NX_Material& material);

    /** Draw call category management */
    static Category category(const NX_Material& material);
    Category category() const;

    /** Internal draw call data */
    const NX_Material& material() const;
    const NX_Mesh& mesh() const;

    /** External draw call data */
    const NX_MaterialShader::TextureArray& materialShaderTextures() const;
    int dynamicRangeIndex() const;
    int drawDataIndex() const;

    /** Draw command */
    void draw(const gpu::Pipeline& pipeline, const NX_InstanceBuffer* instances, int instanceCount) const;

private:
    /** Object to draw */
    NX_Material mMaterial;
    const NX_Mesh& mMesh;

    /** Additionnal data */
    NX_MaterialShader::TextureArray mTextures;  //< Array containing the textures linked to the material shader at the time of draw (if any)
    int mDynamicRangeIndex;                     //< Index of the material shader's dynamic uniform buffer range (if any)
    int mDrawDataIndex;                         //< Index to shared drawing data (DrawData)
};

/* === Container === */

using BucketDrawCalls = util::BucketArray<DrawCall, DrawCall::Category, DrawCall::CATEGORY_COUNT>;

/* === Public Implementation === */

inline DrawCall::DrawCall(int dataIndex, const NX_Mesh& mesh, const NX_Material& material)
    : mMaterial(material), mMesh(mesh), mDynamicRangeIndex(-1), mDrawDataIndex(dataIndex)
{
    if (material.shader != nullptr) {
        mDynamicRangeIndex = material.shader->dynamicRangeIndex();
        material.shader->getTextures(mTextures);
    }
}

inline DrawCall::Category DrawCall::category(const NX_Material& material)
{
    if (material.depth.prePass) {
        return PREPASS;
    }

    if (material.blend != NX_BLEND_OPAQUE) {
        return TRANSPARENT;
    }

    return OPAQUE;
}

inline DrawCall::Category DrawCall::category() const
{
    return category(mMaterial);
}

inline const NX_Material& DrawCall::material() const
{
    return mMaterial;
}

inline const NX_Mesh& DrawCall::mesh() const
{
    return mMesh;
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
    pipeline.bindVertexArray(mMesh.buffer->vao());

    bool useInstancing = (instances && instanceCount > 0);
    bool hasEBO = mMesh.buffer->ebo().isValid();

    if (useInstancing) {
        mMesh.buffer->bindInstances(*instances);
    }

    if (hasEBO) {
        useInstancing ? 
            pipeline.drawElementsInstanced(GL_TRIANGLES, GL_UNSIGNED_INT, mMesh.indexCount, instanceCount) :
            pipeline.drawElements(GL_TRIANGLES, GL_UNSIGNED_INT, mMesh.indexCount);
    } else {
        useInstancing ?
            pipeline.drawInstanced(GL_TRIANGLES, mMesh.vertexCount, instanceCount) :
            pipeline.draw(GL_TRIANGLES, mMesh.vertexCount);
    }

    if (useInstancing) {
        mMesh.buffer->unbindInstances();
    }
}

} // namespace scene

#endif // NX_SCENE_DRAW_CALL_HPP
