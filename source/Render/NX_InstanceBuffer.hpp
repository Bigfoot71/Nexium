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

    /** Buffer mapping */
    void* map(NX_InstanceData type);
    void* mapRange(NX_InstanceData type, size_t offset, size_t count);
    void unmap(NX_InstanceData type);

    /** Get buffer pointer (nullptr if not valid) */
    const gpu::Buffer* getBuffer(NX_InstanceData type) const;
    
    /** Getters */
    NX_InstanceData instanceFlags() const;
    size_t allocatedCount() const;

private:
    static constexpr size_t TypeSizes[] = {
        sizeof(NX_Vec3),
        sizeof(NX_Quat),
        sizeof(NX_Vec3),
        sizeof(NX_Color),
        sizeof(NX_Vec4)
    };

    static constexpr const char* TypeNames[] = {
        "NX_INSTANCE_POSITION",
        "NX_INSTANCE_ROTATION",
        "NX_INSTANCE_SCALE",
        "NX_INSTANCE_COLOR",
        "NX_INSTANCE_CUSTOM"
    };

private:
    std::array<gpu::Buffer, 5> mBuffers{};
    NX_InstanceData mBufferFlags{};
    size_t mAllocatedCount{};
};

/* === Public Implementation === */

inline NX_InstanceBuffer::NX_InstanceBuffer(NX_InstanceData bitfield, size_t count)
    : mBufferFlags(bitfield)
    , mAllocatedCount(count)
{
    helper::forEachBit(bitfield, [this, count](int index) {
        mBuffers[index] = gpu::Buffer(GL_ARRAY_BUFFER, count * TypeSizes[index], nullptr, GL_DYNAMIC_DRAW);
    });
}

inline void NX_InstanceBuffer::update(NX_InstanceData type, size_t offset, size_t count, const void* data)
{
    type = helper::bitScanForward(type);
    gpu::Buffer& buffer = mBuffers[type];

    offset *= TypeSizes[type];
    count *= TypeSizes[type];

    if (!buffer.isValid()) {
        NX_LOG(E, "RENDER: Cannot upload to instance buffer; type '%s' is not initialized.", TypeNames[type]);
        return;
    }

    if (offset + count > buffer.size()) {
        NX_LOG(E, "RENDER: Upload range out of bounds for type '%s' (offset %zu + count %zu > buffer size %zu).",
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

inline void* NX_InstanceBuffer::map(NX_InstanceData type)
{
    type = helper::bitScanForward(type);
    gpu::Buffer& buffer = mBuffers[type];

    if (!buffer.isValid()) {
        NX_LOG(E, "RENDER: Cannot map instance buffer; type '%s' is not initialized.", TypeNames[type]);
        return nullptr;
    }

    return buffer.map(GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

inline void* NX_InstanceBuffer::mapRange(NX_InstanceData type, size_t offset, size_t count)
{
    type = helper::bitScanForward(type);
    gpu::Buffer& buffer = mBuffers[type];

    offset *= TypeSizes[type];
    count *= TypeSizes[type];

    if (!buffer.isValid()) {
        NX_LOG(E, "RENDER: Cannot map instance buffer range; type '%s' is not initialized.", TypeNames[type]);
        return nullptr;
    }

    if (offset + count > buffer.size()) {
        NX_LOG(E, "RENDER: Map range out of bounds for type '%s' (offset %zu + count %zu > buffer size %zu).",
                        TypeNames[type], offset, count, buffer.size());
        return nullptr;
    }

    return buffer.mapRange(offset, count, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
}

inline void NX_InstanceBuffer::unmap(NX_InstanceData type)
{
    mBuffers[helper::bitScanForward(type)].unmap();
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
