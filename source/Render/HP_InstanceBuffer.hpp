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
#include "../Detail/Helper.hpp"

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Math.h>
#include <array>

/* === Declaration === */

class HP_InstanceBuffer {
public:
    HP_InstanceBuffer() = default;

    /** Buffer management */
    void updateBufferData(HP_InstanceData type, const void* data, size_t offset, size_t count, bool keepData);
    void reserveBufferCapacity(HP_InstanceData type, size_t capacity, bool keepData);
    void setBufferState(HP_InstanceData type, bool enabled);

    /** Queries */
    const gpu::Buffer* getBuffer(HP_InstanceData type) const;

private:
    struct BufferInfo {
        gpu::Buffer buffer{};
        bool enabled{false};
    };
    std::array<BufferInfo, 3> mBuffers{};

private:
    static constexpr size_t TypeSizes[3] = {
        sizeof(HP_Mat4),
        sizeof(HP_Color),
        sizeof(HP_Vec4)
    };
};

/* === Public Implementation === */

inline void HP_InstanceBuffer::updateBufferData(HP_InstanceData type, const void* data, size_t offset, size_t count, bool keepData)
{
    type = helper::bitScanForward(type);
    BufferInfo& info = mBuffers[type];

    offset *= TypeSizes[type];
    count *= TypeSizes[type];

    if (!info.buffer.isValid()) {
        info.buffer = gpu::Buffer(GL_ARRAY_BUFFER, offset + count, nullptr, GL_DYNAMIC_DRAW);
    } else {
        info.buffer.reserve(offset + count, keepData);
    }

    info.buffer.upload(offset, count, data);
}

inline void HP_InstanceBuffer::reserveBufferCapacity(HP_InstanceData bitfield, size_t count, bool keepData)
{
    helper::forEachBit(static_cast<uint32_t>(bitfield), [&](int index) {
        if (!mBuffers[index].buffer.isValid()) {
            mBuffers[index].buffer = gpu::Buffer(GL_ARRAY_BUFFER, count * TypeSizes[index], nullptr, GL_DYNAMIC_DRAW);
        } else {
            mBuffers[index].buffer.reserve(count * TypeSizes[index], keepData);
        }
    });
}

inline void HP_InstanceBuffer::setBufferState(HP_InstanceData bitfield, bool enabled)
{
    helper::forEachBit(static_cast<uint32_t>(bitfield), [&](int index) {
        mBuffers[index].enabled = enabled;
    });
}

inline const gpu::Buffer* HP_InstanceBuffer::getBuffer(HP_InstanceData type) const
{
    return &mBuffers[helper::bitScanForward(type)].buffer;
}

#endif // HP_INSTANCE_BUFFER_HPP
