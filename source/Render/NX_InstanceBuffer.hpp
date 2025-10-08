/* NX_InstanceBuffer.hpp -- Implementation of the API for instance buffers
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_INSTANCE_BUFFER_HPP
#define NX_INSTANCE_BUFFER_HPP

#include "../Detail/GPU/Buffer.hpp"
#include "../Detail/Helper.hpp"

#include <NX/NX_Render.h>
#include <NX/NX_Math.h>
#include <array>

/* === Declaration === */

class NX_InstanceBuffer {
public:
    NX_InstanceBuffer() = default;

    /** Buffer management */
    void updateBufferData(NX_InstanceData type, const void* data, size_t offset, size_t count, bool keepData);
    void reserveBufferCapacity(NX_InstanceData type, size_t capacity, bool keepData);
    void setBufferState(NX_InstanceData type, bool enabled);

    /** Get buffer pointer, and if enabled (null otherwise) */
    const gpu::Buffer* getBuffer(NX_InstanceData type) const;
    const gpu::Buffer* getEnabledBuffer(NX_InstanceData type) const;

private:
    struct BufferInfo {
        gpu::Buffer buffer{};
        bool enabled{false};
    };
    std::array<BufferInfo, 3> mBuffers{};

private:
    static constexpr size_t TypeSizes[3] = {
        sizeof(NX_Mat4),
        sizeof(NX_Color),
        sizeof(NX_Vec4)
    };
};

/* === Public Implementation === */

inline void NX_InstanceBuffer::updateBufferData(NX_InstanceData type, const void* data, size_t offset, size_t count, bool keepData)
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

inline void NX_InstanceBuffer::reserveBufferCapacity(NX_InstanceData bitfield, size_t count, bool keepData)
{
    helper::forEachBit(static_cast<uint32_t>(bitfield), [&](int index) {
        if (!mBuffers[index].buffer.isValid()) {
            mBuffers[index].buffer = gpu::Buffer(GL_ARRAY_BUFFER, count * TypeSizes[index], nullptr, GL_DYNAMIC_DRAW);
        } else {
            mBuffers[index].buffer.reserve(count * TypeSizes[index], keepData);
        }
    });
}

inline void NX_InstanceBuffer::setBufferState(NX_InstanceData bitfield, bool enabled)
{
    helper::forEachBit(static_cast<uint32_t>(bitfield), [&](int index) {
        mBuffers[index].enabled = enabled;
    });
}

inline const gpu::Buffer* NX_InstanceBuffer::getBuffer(NX_InstanceData type) const
{
    return &mBuffers[helper::bitScanForward(type)].buffer;
}

inline const gpu::Buffer* NX_InstanceBuffer::getEnabledBuffer(NX_InstanceData type) const
{
    if (mBuffers[helper::bitScanForward(type)].enabled) {
        return getBuffer(type);
    }
    return nullptr;
}

#endif // NX_INSTANCE_BUFFER_HPP
