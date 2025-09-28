/* DrawData.hpp -- Represent drawing data that can be shared by multiple draw calls
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_SCENE_DRAW_DATA_HPP
#define HP_SCENE_DRAW_DATA_HPP

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Math.h>

#include "../../Detail/Util/DynamicArray.hpp"

namespace scene {

/* === Declaration === */

class DrawData {
public:
    DrawData(
        const HP_Transform& transform,
        const HP_InstanceBuffer* instances = nullptr,
        int instanceCount = 0, int boneMatrixOffset = -1
    );

    /** Transform */
    const HP_Transform& transform() const;
    const HP_Mat4& matrix() const;
    const HP_Mat3& normal() const;

    /** Instances */
    const HP_InstanceBuffer* instances() const;
    int instanceCount() const;
    bool useInstancing() const;

    /** Animations */
    int boneMatrixOffset() const;
    bool isAnimated() const;

private:
    /** Transform */
    HP_Transform mTransform;
    HP_Mat4 mMatrix;
    HP_Mat3 mNormal;

    /** Instances */
    const HP_InstanceBuffer* mInstances;
    int mInstanceCount;

    /** Animations */
    int mBoneMatrixOffset;  //< If less than zero, no animation assigned
};

/* === Container === */

using ArrayDrawData = util::DynamicArray<DrawData>;

/* === Public Implementation === */

inline DrawData::DrawData(const HP_Transform& transform, const HP_InstanceBuffer* instances, int instanceCount, int boneMatrixOffset)
    : mTransform(transform)
    , mMatrix(HP_TransformToMat4(&transform))
    , mNormal(HP_Mat3Normal(&mMatrix))
    , mInstances(instances)
    , mInstanceCount(instanceCount)
    , mBoneMatrixOffset(boneMatrixOffset)
{ }

inline const HP_Transform& DrawData::transform() const
{
    return mTransform;
}

inline const HP_Mat4& DrawData::matrix() const
{
    return mMatrix;
}

inline const HP_Mat3& DrawData::normal() const
{
    return mNormal;
}

inline const HP_InstanceBuffer* DrawData::instances() const
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

inline bool DrawData::isAnimated() const
{
    return (mBoneMatrixOffset >= 0);
}

} // namespace scene

#endif // HP_SCENE_DRAW_DATA_HPP
