/* DrawCall.hpp -- Represents a draw call for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_SCENE_DRAW_CALL_HPP
#define HP_SCENE_DRAW_CALL_HPP

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Math.h>

#include "../../Detail/Util/BucketArray.hpp"
#include "../../Detail/GPU/Pipeline.hpp"
#include "../HP_MaterialShader.hpp"
#include "../HP_VertexBuffer.hpp"

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
    DrawCall(int dataIndex, const HP_Mesh& mesh, const HP_Material& material);

    /** Draw call category management */
    static Category category(const HP_Material& material);
    Category category() const;

    /** Internal draw call data */
    const HP_Material& material() const;
    const HP_Mesh& mesh() const;

    /** External draw call data */
    const HP_MaterialShader::TextureArray& materialShaderTextures() const;
    int dynamicRangeIndex() const;
    int drawDataIndex() const;

    /** Draw command */
    void draw(const gpu::Pipeline& pipeline, const HP_InstanceBuffer* instances, int instanceCount) const;

private:
    /** Object to draw */
    HP_Material mMaterial;
    const HP_Mesh& mMesh;

    /** Additionnal data */
    HP_MaterialShader::TextureArray mTextures;  //< Array containing the textures linked to the material shader at the time of draw (if any)
    int mDynamicRangeIndex;                     //< Index of the material shader's dynamic uniform buffer range (if any)
    int mDrawDataIndex;                         //< Index to shared drawing data (DrawData)
};

/* === Container === */

using BucketDrawCalls = util::BucketArray<DrawCall, DrawCall::Category, DrawCall::CATEGORY_COUNT>;

/* === Public Implementation === */

inline DrawCall::DrawCall(int dataIndex, const HP_Mesh& mesh, const HP_Material& material)
    : mMaterial(material), mMesh(mesh), mDynamicRangeIndex(-1), mDrawDataIndex(dataIndex)
{
    if (material.shader != nullptr) {
        mDynamicRangeIndex = material.shader->dynamicRangeIndex();
        material.shader->getTextures(mTextures);
    }
}

inline DrawCall::Category DrawCall::category(const HP_Material& material)
{
    if (material.depth.prePass) {
        return PREPASS;
    }

    if (material.blend != HP_BLEND_OPAQUE) {
        return TRANSPARENT;
    }

    return OPAQUE;
}

inline DrawCall::Category DrawCall::category() const
{
    return category(mMaterial);
}

inline const HP_Material& DrawCall::material() const
{
    return mMaterial;
}

inline const HP_Mesh& DrawCall::mesh() const
{
    return mMesh;
}

inline const HP_MaterialShader::TextureArray& DrawCall::materialShaderTextures() const
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

inline void DrawCall::draw(const gpu::Pipeline& pipeline, const HP_InstanceBuffer* instances, int instanceCount) const
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

#endif // HP_SCENE_DRAW_CALL_HPP
