/* NX_InstanceBuffer.cpp -- API definition for Nexium's instance buffer module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_InstanceBuffer.hpp"
#include "./INX_PoolAssets.hpp"

// ============================================================================
// PUBLIC API
// ============================================================================

NX_InstanceBuffer* NX_CreateInstanceBuffer(NX_InstanceData bitfield, size_t count)
{
    NX_InstanceBuffer* buffer = INX_Pool.Create<NX_InstanceBuffer>();

    buffer->bufferFlags = bitfield;
    buffer->allocatedCount = count;

    helper::ForEachBit(bitfield, [buffer, count](int index) {
        size_t size = count * NX_InstanceBuffer::TypeSizes[index];
        buffer->buffers[index] = gpu::Buffer(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    });

    return buffer;
}

void NX_DestroyInstanceBuffer(NX_InstanceBuffer* buffer)
{
    INX_Pool.Destroy(buffer);
}

void NX_RaeallocInstanceBuffer(NX_InstanceBuffer* buffer, size_t count, bool keepData)
{
    for (int i = 0; i < buffer->buffers.size(); i++) {
        if (buffer->buffers[i].isValid()) {
            size_t size = count * NX_InstanceBuffer::TypeSizes[i];
            buffer->buffers[i].realloc(size, keepData);
        }
    }

    buffer->allocatedCount = count;
}

void NX_UpdateInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type, size_t offset, size_t count, const void* data)
{
    type = helper::BitScanForward(type);
    gpu::Buffer& gpu = buffer->buffers[type];

    offset *= NX_InstanceBuffer::TypeSizes[type];
    count *= NX_InstanceBuffer::TypeSizes[type];

    if (!gpu.isValid()) {
        NX_LOG(E, "RENDER: Cannot upload to instance buffer; type '%s' is not initialized.", NX_InstanceBuffer::TypeNames[type]);
        return;
    }

    if (offset + count > gpu.size()) {
        NX_LOG(E, "RENDER: Upload range out of bounds for type '%s' (offset %zu + count %zu > buffer size %zu).",
            NX_InstanceBuffer::TypeNames[type], offset, count, gpu.size());
        return;
    }

    gpu.upload(offset, count, data);
}

void* NX_MapInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type)
{
    type = helper::BitScanForward(type);
    gpu::Buffer& gpu = buffer->buffers[type];

    if (!gpu.isValid()) {
        NX_LOG(E, "RENDER: Cannot map instance buffer; type '%s' is not initialized.", NX_InstanceBuffer::TypeNames[type]);
        return nullptr;
    }

    return gpu.map(GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void* NX_MapInstanceBufferRange(NX_InstanceBuffer* buffer, NX_InstanceData type, size_t offset, size_t count)
{
    type = helper::BitScanForward(type);
    gpu::Buffer& gpu = buffer->buffers[type];

    offset *= NX_InstanceBuffer::TypeSizes[type];
    count *= NX_InstanceBuffer::TypeSizes[type];

    if (!gpu.isValid()) {
        NX_LOG(E, "RENDER: Cannot map instance buffer range; type '%s' is not initialized.", NX_InstanceBuffer::TypeNames[type]);
        return nullptr;
    }

    if (offset + count > gpu.size()) {
        NX_LOG(E, "RENDER: Map range out of bounds for type '%s' (offset %zu + count %zu > buffer size %zu).",
            NX_InstanceBuffer::TypeNames[type], offset, count, gpu.size());
        return nullptr;
    }

    return gpu.mapRange(offset, count, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
}

void NX_UnmapInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData type)
{
    buffer->buffers[helper::BitScanForward(type)].unmap();
}

void NX_QueryInstanceBuffer(NX_InstanceBuffer* buffer, NX_InstanceData* bitfield, size_t* count)
{
    if (bitfield) *bitfield = buffer->bufferFlags;
    if (count) *count = buffer->allocatedCount;
}
