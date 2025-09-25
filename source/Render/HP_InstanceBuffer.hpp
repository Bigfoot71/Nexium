/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef HP_INSTANCE_BUFFER_HPP
#define HP_INSTANCE_BUFFER_HPP

#include "../Detail/GPU/Buffer.hpp"
#include <Hyperion/HP_Math.h>

/* === Declaration === */

class HP_InstanceBuffer {
public:
    HP_InstanceBuffer() = default;

    void setData(const HP_Mat4* matrices, const HP_Color* colors, const HP_Vec4* custom, int count);

    const gpu::Buffer* matrices() const;
    const gpu::Buffer* colors() const;
    const gpu::Buffer* custom() const;

private:
    gpu::Buffer mMatrices{};    //< HP_Mat4
    gpu::Buffer mColors{};      //< HP_Color
    gpu::Buffer mCustom{};      //< HP_Vec4

private:
    bool mHasMatrices{};
    bool mHasColors{};
    bool mHasCustom{};
};

/* === Public Implementation === */

inline void HP_InstanceBuffer::setData(const HP_Mat4* matrices, const HP_Color* colors, const HP_Vec4* custom, int count)
{
    mHasMatrices = (matrices != nullptr);
    mHasColors = (colors != nullptr);
    mHasCustom = (custom != nullptr);

    if (mHasMatrices) {
        if (mMatrices.isValid()) {
            mMatrices.reserve(sizeof(HP_Mat4) * count, false);
            mMatrices.upload(matrices);
        }
        else {
            mMatrices = gpu::Buffer(
                GL_ARRAY_BUFFER, sizeof(HP_Mat4) * count,
                matrices, GL_DYNAMIC_DRAW
            );
        }
    }

    if (mHasColors) {
        if (mColors.isValid()) {
            mColors.reserve(sizeof(HP_Color) * count, false);
            mColors.upload(colors);
        }
        else {
            mColors = gpu::Buffer(
                GL_ARRAY_BUFFER, sizeof(HP_Color) * count,
                colors, GL_DYNAMIC_DRAW
            );
        }
    }

    if (mHasCustom) {
        if (mCustom.isValid()) {
            mCustom.reserve(sizeof(HP_Vec4) * count, false);
            mCustom.upload(custom);
        }
        else {
            mCustom = gpu::Buffer(
                GL_ARRAY_BUFFER, sizeof(HP_Vec4) * count,
                custom, GL_DYNAMIC_DRAW
            );
        }
    }
}

inline const gpu::Buffer* HP_InstanceBuffer::matrices() const
{
    return mHasMatrices ? &mMatrices : nullptr;
}

inline const gpu::Buffer* HP_InstanceBuffer::colors() const
{
    return mHasColors ? &mColors : nullptr;
}

inline const gpu::Buffer* HP_InstanceBuffer::custom() const
{
    return mHasCustom ? &mCustom : nullptr;
}

#endif // HP_INSTANCE_BUFFER_HPP
