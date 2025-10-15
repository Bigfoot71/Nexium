/* DrawData.hpp -- Represent drawing data that can be shared by multiple draw calls
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_DRAW_DATA_HPP
#define NX_SCENE_DRAW_DATA_HPP

#include <NX/NX_Render.h>
#include <NX/NX_Math.h>

#include "../../Detail/Util/DynamicArray.hpp"

namespace scene {

/* === Declaration === */

class DrawData {
public:
    DrawData(
        int modelDataIndex,
        const NX_Transform& transform,
        const NX_InstanceBuffer* instances = nullptr,
        int instanceCount = 0, int boneMatrixOffset = -1
    );

    /** Transform */
    const NX_Transform& transform() const;

    /** Instances */
    const NX_InstanceBuffer* instances() const;
    int instanceCount() const;
    bool useInstancing() const;

    /** Animations */
    int boneMatrixOffset() const;
    bool useSkinning() const;

    /** Model data (SSBO) */
    int modelDataIndex() const;

private:
    /** Transform */
    NX_Transform mTransform;

    /** Instances */
    const NX_InstanceBuffer* mInstances;
    int mInstanceCount;

    /** Animations */
    int mBoneMatrixOffset;  //< If less than zero, no animation assigned

    /** Model data (SSBO) */
    int mModelDataIndex;
};

/* === Container === */

using ArrayDrawData = util::DynamicArray<DrawData>;

/* === Public Implementation === */

inline DrawData::DrawData(
    int modelDataIndex,
    const NX_Transform& transform,
    const NX_InstanceBuffer* instances,
    int instanceCount, int boneMatrixOffset)
    : mTransform(transform)
    , mInstances(instances)
    , mInstanceCount(instanceCount)
    , mBoneMatrixOffset(boneMatrixOffset)
    , mModelDataIndex(modelDataIndex)
{ }

inline const NX_Transform& DrawData::transform() const
{
    return mTransform;
}

inline const NX_InstanceBuffer* DrawData::instances() const
{
    return mInstances;
}

inline int DrawData::instanceCount() const
{
    return mInstanceCount;
}

inline bool DrawData::useInstancing() const
{
    return (mInstances != nullptr && mInstanceCount > 0);
}

inline int DrawData::boneMatrixOffset() const
{
    return mBoneMatrixOffset;
}

inline bool DrawData::useSkinning() const
{
    return (mBoneMatrixOffset >= 0);
}

inline int DrawData::modelDataIndex() const
{
    return mModelDataIndex;
}

} // namespace scene

#endif // NX_SCENE_DRAW_DATA_HPP
