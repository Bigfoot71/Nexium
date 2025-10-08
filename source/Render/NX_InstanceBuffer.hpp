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
    /** Constructors */
    NX_InstanceBuffer(NX_InstanceData bitfield, size_t count);

    /** Buffer management */
    void update(NX_InstanceData type, size_t offset, size_t count, const void* data);
    void realloc(size_t capacity, bool keepData);

    /** Get buffer pointer (nullptr if not valid) */
    const gpu::Buffer* getBuffer(NX_InstanceData type) const;
    
    /** Getters */
    NX_InstanceData instanceFlags() const;
    size_t allocatedCount() const;

private:
    static constexpr size_t TypeSizes[3] = {
        sizeof(NX_Mat4),
        sizeof(NX_Color),
        sizeof(NX_Vec4)
    };

    static constexpr const char* TypeNames[] = {
        "NX_INSTANCE_DATA_MATRIX",
        "NX_INSTANCE_DATA_COLOR",
        "NX_INSTANCE_DATA_CUSTOM"
    };

private:
    std::array<gpu::Buffer, 3> mBuffers{};
    NX_InstanceData mBufferFlags{};
    size_t mAllocatedCount{};
};

/* === Public Implementation === */

inline NX_InstanceBuffer::NX_InstanceBuffer(NX_InstanceData bitfield, size_t count)
    : mBufferFlags(bitfield)
    , mAllocatedCount(count)
{
    for (int i = 0; i < mBuffers.size(); i++) {
        if (bitfield & helper::bitScanReverse(i)) {
            mBuffers[i] = gpu::Buffer(GL_ARRAY_BUFFER, count * TypeSizes[i], nullptr, GL_DYNAMIC_DRAW);
        }
    }
}

inline void NX_InstanceBuffer::update(NX_InstanceData type, size_t offset, size_t count, const void* data)
{
    type = helper::bitScanForward(type);
    gpu::Buffer& buffer = mBuffers[type];

    offset *= TypeSizes[type];
    count *= TypeSizes[type];

    if (!buffer.isValid()) {
        NX_INTERNAL_LOG(E, "RENDER: Cannot upload to instance buffer; type '%s' is not initialized.", TypeNames[type]);
        return;
    }

    if (offset + count > buffer.size()) {
        NX_INTERNAL_LOG(E, "RENDER: Upload range out of bounds for type '%s' (offset %zu + count %zu > buffer size %zu).",
                        TypeNames[type], offset, count, buffer.size());
        return;
    }

    buffer.upload(offset, count, data);
}

inline void NX_InstanceBuffer::realloc(size_t count, bool keepData)
{
    for (int i = 0; i < mBuffers.size(); i++) {
        if (mBuffers[i].isValid()) {
            mBuffers[i].realloc(count * TypeSizes[i], keepData);
        }
    }

    mAllocatedCount = count;
}

inline const gpu::Buffer* NX_InstanceBuffer::getBuffer(NX_InstanceData type) const
{
    const gpu::Buffer& buffer = mBuffers[helper::bitScanForward(type)];
    return buffer.isValid() ? &buffer : nullptr;
}

inline NX_InstanceData NX_InstanceBuffer::instanceFlags() const
{
    return mBufferFlags;
}

inline size_t NX_InstanceBuffer::allocatedCount() const
{
    return mAllocatedCount;
}

#endif // NX_INSTANCE_BUFFER_HPP
