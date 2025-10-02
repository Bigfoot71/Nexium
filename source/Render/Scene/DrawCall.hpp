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

    /** Draw call data */
    const HP_Material& material() const;
    const HP_Mesh& mesh() const;
    int dataIndex() const;

    /** Draw command */
    void draw(const gpu::Pipeline& pipeline, const HP_InstanceBuffer* instances, int instanceCount) const;

private:
    HP_Material mMaterial;
    const HP_Mesh& mMesh;
    int mDataIndex;
};

/* === Container === */

using BucketDrawCalls = util::BucketArray<DrawCall, DrawCall::Category, DrawCall::CATEGORY_COUNT>;

/* === Public Implementation === */

inline DrawCall::DrawCall(int dataIndex, const HP_Mesh& mesh, const HP_Material& material)
    : mMaterial(material), mMesh(mesh), mDataIndex(dataIndex)
{ }

inline DrawCall::Category DrawCall::category(const HP_Material& material)
{
    if (material.depthPrePass) {
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

inline int DrawCall::dataIndex() const
{
    return mDataIndex;
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
